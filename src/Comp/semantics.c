/* Semantics.c
   Support and semantic action routines.
*/

#include "semantics.h"

/* Semantics support routines */

struct VarType*
doVarType( int type )
{
    struct VarType* nType = malloc( sizeof( struct VarType ) );
    nType->Type = type;
    nType->Size = 1;
    nType->isRef = 0;
    return nType;
}

void typeMismatch()
{
    WriteIndicator( GetCurrentColumn() );
    WriteMessage( "Type Mismatch" );
    exit( 1 );
}

void doDeclare( char* name, struct VarType* type, int arg, int size )
{
    struct SymEntry* ent;
    type->isArg = arg;
    type->SPos = sPos;
    type->Shim = NULL;
    sPos += type->Size * 4;
    if ( arg )
    {
        type->ArgPos = paramPos;
        paramPos++;
    }
    if ( tabList )
    {
        type->Loc = V_LOC;
    }
    else
    {
        type->Loc = V_GBL;
    }
    if ( size > 1 )
    {
        type->isRef = 1;
        char buf[ 50 ];
        snprintf( buf, 50, "_%s", name );
        type->Shim = strdup( buf );
        struct VarType* shim = malloc( sizeof( struct VarType ) );
        memcpy( shim, type, sizeof( struct VarType ) );
        shim->Size = size;
        shim->Shim = NULL;
        shim->SPos = sPos;
        sPos += shim->Size * 4;
        EnterName( curTab, buf, &ent );
        SetAttr( ent, (void*)shim );
    }
    EnterName( curTab, name, &ent );
    SetAttr( ent, (void*)type );
}

void doPushDecs()
{
    struct TabList* nList = malloc( sizeof( struct TabList ) );
    nList->Tab = curTab;
    nList->Next = tabList;
    tabList = nList;
    curTab = CreateSymTab( 33 );
}

void doPopDecs()
{
    struct TabList* nHead = tabList->Next;
    free( tabList );
    tabList = nHead;
    sPos = 0;
    paramPos = 0;
}

struct VarType*
doFindVar( char* name )
{
    struct VarType* vType = NULL;
    struct TabList* tEnt = tabList;
    while ( tEnt )
    {
        struct SymEntry* ent = FindName( tEnt->Tab, name );
        if ( ent )
        {
            return (struct VarType*)GetAttr( ent );
        }
        tEnt = tEnt->Next;
    }
    return NULL;
}

struct ExprRes*
doRval( struct IdAddr* addr )
{
    struct ExprRes* addrExpr = addr->Addr;
    AppendSeq( addrExpr->Instrs, GenInstr( NULL, "lw",
                                     TmpRegName( addrExpr->Reg ),
                                     RegOff( 0, TmpRegName( addrExpr->Reg ) ),
                                     NULL ) );
    free( addr->Name );
    free( addr );
    return addrExpr;
}

struct InstrSeq*
doPrintSpaceHlp()
{
    struct InstrSeq* code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_sp", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq*
doPrintLn()
{
    struct InstrSeq* code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_nl", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq*
doPrintList( struct ExprRes* Res1, struct InstrSeq* instrs )
{
    struct InstrSeq* res;
    res = doPrint( Res1 );
    AppendSeq( res, doPrintSpaceHlp() );
    AppendSeq( res, instrs );
    return res;
}

struct InstrSeq*
doPrint( struct ExprRes* Expr )
{
    if ( !Expr->Type->isRef )
    {
        if ( Expr->Type->Type == T_BOOL )
            return doPrintBool( Expr );
        if ( Expr->Type->Type == T_INT )
            return doPrintInt( Expr );
        if ( Expr->Type->Type == T_STR )
            return doPrintStr( Expr );
    }
    typeMismatch();
    return NULL;
}

struct InstrSeq*
doPrintSp( struct ExprRes* Expr )
{
    if ( Expr->Type->Type != T_INT || Expr->Type->isRef )
    {
        typeMismatch();
    }

    struct InstrSeq* code;
    int reg_count = AvailTmpReg();
    int reg_one = AvailTmpReg();
    char* s_label = GenLabel();
    char* e_label = GenLabel();
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "addi",
                         TmpRegName( reg_one ),
                         "$0", "1" ) );
    AppendSeq( code, GenInstr( NULL, "blt",
                         TmpRegName( Expr->Reg ),
                         TmpRegName( reg_one ),
                         e_label ) );
    AppendSeq( code, GenInstr( NULL, "move",
                         TmpRegName( reg_count ),
                         "$0", NULL ) );
    AppendSeq( code, GenInstr( s_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( code, doPrintSpaceHlp() );
    AppendSeq( code, GenInstr( NULL, "add",
                         TmpRegName( reg_count ),
                         TmpRegName( reg_count ),
                         TmpRegName( reg_one ) ) );
    AppendSeq( code, GenInstr( NULL, "blt",
                         TmpRegName( reg_count ),
                         TmpRegName( Expr->Reg ),
                         s_label ) );
    AppendSeq( code, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    ReleaseTmpReg( reg_count );
    ReleaseTmpReg( reg_one );
    free( Expr->Type );
    free( Expr );
    free( s_label );
    free( e_label );
    return code;
}

struct InstrSeq*
doAssign( struct IdAddr* addr, struct ExprRes* Expr, int inverse )
{
    struct InstrSeq* code;
    struct ExprRes* addrExpr = addr->Addr;
    if ( Expr->Type->Type != T_ANY && ( ( addrExpr->Type->Type != Expr->Type->Type )
                                          || ( addrExpr->Type->isRef != Expr->Type->isRef ) ) )
    {
        typeMismatch();
    }
    if ( inverse )
    {
        code = Expr->Instrs;
        AppendSeq( code, addrExpr->Instrs );
    }
    else
    {
        code = addrExpr->Instrs;
        AppendSeq( code, Expr->Instrs );
    }
    AppendSeq( code, GenInstr( NULL, "sw",
                         TmpRegName( Expr->Reg ),
                         RegOff( 0, TmpRegName( addrExpr->Reg ) ),
                         NULL ) );
    ReleaseTmpReg( Expr->Reg );
    ReleaseTmpReg( addrExpr->Reg );
    free( addr->Name );
    free( addr->Addr->Type );
    free( addr->Addr );
    free( addr );
    free( Expr->Type );
    free( Expr );
    return code;
}

struct InstrSeq*
doRead( struct IdAddr* addr )
{
    struct ExprRes* res = malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Type = malloc( sizeof( struct VarType ) );
    memcpy( res->Type, addr->Addr->Type, sizeof( struct VarType ) );
    res->Instrs = GenInstr( NULL, "li", "$v0", "5", NULL );
    AppendSeq( res->Instrs, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( res->Instrs, GenInstr( NULL, "move",
                                TmpRegName( res->Reg ),
                                "$v0", NULL ) );
    return doAssign( addr, res, 0 );
}

void Finish( struct InstrSeq* Code )
{
    struct InstrSeq* code;
    struct SymEntry* entry;
    struct Attr* attr;
    struct StrLitList* strEnt;
    code = GenInstr( NULL, ".text", NULL, NULL, NULL );
    AppendSeq( code, GenInstr( NULL, ".globl", "main", NULL, NULL ) );
    AppendSeq( code, GenInstr( "main", NULL, NULL, NULL, NULL ) );
    AppendSeq( code, doFuncInstrs( doCall( "main" ) ) );
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "10", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, Code );
    AppendSeq( code, GenInstr( NULL, ".data", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_nl", ".asciiz", "\"\\n\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_true", ".asciiz", "\"true\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_false", ".asciiz", "\"false\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_sp", ".asciiz", "\" \"", NULL, NULL ) );

    entry = FirstEntry( tabList->Tab );
    while ( entry )
    {
        AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
        struct VarType* vType;
        vType = GetAttr( entry );
        char buf[ 50 ];
        if ( vType->Shim )
        {
            AppendSeq( code, GenInstr( (char*)GetName( entry ), ".word", vType->Shim, NULL, NULL ) );
        }
        else
        {
            snprintf( buf, 50, "%d", vType->Size * 4 );
            AppendSeq( code, GenInstr( (char*)GetName( entry ), ".space", buf, NULL, NULL ) );
        }
        entry = NextEntry( tabList->Tab, entry );
    }

    strEnt = strList;
    while ( strEnt )
    {
        AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
        AppendSeq( code, GenInstr( strEnt->label, ".asciiz", strEnt->val, NULL, NULL ) );
        strEnt = strEnt->next;
    }

    WriteSeq( code );
}

