/* Semantics.c
   Support and semantic action routines.
*/

#include "semantics.h"

/* Semantics support routines */

struct VarType *
doVarType(int type)
{
    struct VarType *nType = malloc(sizeof(struct VarType));
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

void doDeclare( char* name, struct VarType *type, int arg )
{
    struct SymEntry* ent;
    type->Arg = arg;
    type->SPos = sPos;
    sPos += type->Size * 4;
    if( arg )
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
doRval( char* name )
{
    struct ExprRes* res;
    struct VarType* vType = doFindVar( name );
    if ( !vType )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared Variable" );
        exit( 1 );
    }
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    if ( vType->Loc == V_GBL )
    {
        res->Instrs = GenInstr( NULL, "lw",
            TmpRegName( res->Reg ),
            name, NULL );
    }
    else
    {
        res->Instrs = GenInstr( NULL, "lw",
            TmpRegName( res->Reg ),
            RegOff( vType->SPos, "$sp" ),
            NULL );
    }
    res->Type = malloc(sizeof(struct VarType));
    memcpy(res->Type, vType, sizeof(struct VarType));
    return res;
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
    if ( Expr->Type->Type != T_INT || Expr->Type->isRef)
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
doAssign( char* name, struct ExprRes* Expr, int SZOff )
{
    struct InstrSeq* code;
    struct VarType* vType = doFindVar( name );
    if ( !vType )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit( 1 );
    }
    printf("%d\n", Expr->Type->isRef);
    printf("%d\n", vType->isRef);
    if ( Expr->Type->Type != T_ANY && ((vType->Type != Expr->Type->Type)
            || (vType->isRef != Expr->Type->isRef)))
    {
        typeMismatch();
    }
    code = Expr->Instrs;
    if ( vType->Loc == V_GBL )
    {
        AppendSeq( code, GenInstr( NULL, "sw",
                             TmpRegName( Expr->Reg ),
                             name, NULL ) );
    }
    else
    {
        if( !SZOff )
        {
            AppendSeq( code, GenInstr( NULL, "sw",
                             TmpRegName( Expr->Reg ),
                             RegOff( vType->SPos, "$sp" ),
                             NULL ) );
        }
        else
        {
            AppendSeq( code, GenInstr( NULL, "sw",
                             TmpRegName( Expr->Reg ),
                             RegOff( vType->SPos, "$s0" ),
                             NULL ) );
        }
    }
    ReleaseTmpReg( Expr->Reg );
    free( Expr->Type );
    free( Expr );
    return code;
}

struct InstrSeq*
doRead( char* var )
{
    struct ExprRes* res = malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Type = doVarType(T_ANY);
    res->Instrs = GenInstr( NULL, "li", "$v0", "5", NULL );
    AppendSeq( res->Instrs, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( res->Instrs, GenInstr( NULL, "move",
                                TmpRegName( res->Reg ),
                                "$v0", NULL ) );
    return doAssign( var, res, 0 );
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
        snprintf( buf, 50, "%d", vType->Size * 4 );
        AppendSeq( code, GenInstr( (char*)GetName( entry ), ".space", buf, NULL, NULL ) );
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

