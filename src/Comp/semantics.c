/* Semantics.c
   Support and semantic action routines.
*/

#include "semantics.h"

/* Semantics support routines */

int parseStdLib = 0;
int lib_pos = 0;
struct InstrSeq *stdLibInstrs;

int getLexInput()
{
    if(!parseStdLib)
    {
        return GetSourceChar();
    }
    if(lib_pos >= libclite_cl_len)
    {
        return EOF;
    }
    return libclite_cl[lib_pos++];
}

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
    return doFuncInstrs(doCall("INTERNALPrintLn"));
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
        if ( Expr->Type->Type == T_FLOAT )
            return doPrintFloat( Expr );
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
    doDecArg(Expr);
    return doFuncInstrs(doCall("INTERNALPrintSp"));
}

struct InstrSeq*
doAssign( struct IdAddr* addr, struct ExprRes* Expr, int inverse )
{
    struct InstrSeq* code;
    struct ExprRes* addrExpr = addr->Addr;
    if ( addrExpr->Type->Type != Expr->Type->Type
        || addrExpr->Type->isRef != Expr->Type->isRef )
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
    struct ExprRes* res;
    switch ( addr->Addr->Type->Type )
    {
    case T_INT:
    case T_BOOL:
        res = doReadInt( addr );
        break;
    case T_FLOAT:
        res = doReadFloat( addr );
        break;
    default:
        typeMismatch();
    }
    return doAssign( addr, res, 0 );
}

void Finish( struct InstrSeq* Code )
{
    if(parseStdLib)
    {
        stdLibInstrs = Code;
        return;
    }
    
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
    AppendSeq( code, stdLibInstrs );
    AppendSeq( code, Code );
    AppendSeq( code, GenInstr( NULL, ".data", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
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

struct ExprRes* doComp( struct ExprRes* Res1, struct ExprRes* Res2, int op )
{
    if ( Res1->Type->isRef || Res2->Type->isRef )
    {
        typeMismatch();
    }
    int res1_type = Res1->Type->Type;
    int res2_type = Res2->Type->Type;
    if ( res1_type == T_INT && res2_type == T_INT )
    {
        return doCompInt( Res1, Res2, op );
    }
    if ( res1_type == T_FLOAT && res2_type == T_FLOAT )
    {
        return doCompFloat( Res1, Res2, op );
    }
    if ( res1_type == T_INT )
    {
        Res1 = doIntToFloat( Res1 );
        return doComp( Res1, Res2, op );
    }
    if ( res2_type == T_INT )
    {
        Res2 = doIntToFloat( Res2 );
        return doComp( Res1, Res2, op );
    }
    typeMismatch();
    return NULL;
}

struct ExprRes* doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op )
{
    if ( Res1->Type->isRef || Res2->Type->isRef )
    {
        typeMismatch();
    }
    int res1_type = Res1->Type->Type;
    int res2_type = Res2->Type->Type;
    if ( res1_type == T_INT && res2_type == T_INT )
    {
        return doArithInt( Res1, Res2, op );
    }
    if ( res1_type == T_FLOAT && res2_type == T_FLOAT )
    {
        return doArithFloat( Res1, Res2, op );
    }
    if ( res1_type == T_INT )
    {
        Res1 = doIntToFloat( Res1 );
        return doArith( Res1, Res2, op );
    }
    if ( res2_type == T_INT )
    {
        Res2 = doIntToFloat( Res2 );
        return doArith( Res1, Res2, op );
    }
    typeMismatch();
    return NULL;
}

struct ExprRes* doPow( struct ExprRes* base, struct ExprRes* pow )
{
    if ( base->Type->isRef || pow->Type->isRef )
    {
        typeMismatch();
    }
    int base_type = base->Type->Type;
    int pow_type = pow->Type->Type;
    if (pow_type == T_FLOAT)
    {
        pow = doFloatToInt(pow);
        pow_type = T_INT;
    }
    if ( base_type == T_INT && pow_type == T_INT )
    {
        return doPowInt( base, pow );
    }
    if (base_type == T_FLOAT && pow_type == T_INT)
    {
        return doPowFloat(base, pow);
    }
    typeMismatch();
    return NULL;
}

struct ExprRes* doNegate( struct ExprRes* Expr )
{
    if ( Expr->Type->isRef )
    {
        typeMismatch();
    }
    int expr_type = Expr->Type->Type;
    if ( expr_type == T_INT )
    {
        return doNegateInt( Expr );
    }
    if ( expr_type == T_FLOAT )
    {
        return doNegateFloat( Expr );
    }
    typeMismatch();
    return NULL;
}
