#include "semantics.h"

struct ExprRes*
doBoolLit( int b )
{
    struct ExprRes* res;
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    if ( b )
    {
        res->Instrs = GenInstr( NULL, "li", TmpRegName( res->Reg ), "1", NULL );
    }
    else
    {
        res->Instrs = GenInstr( NULL, "li", TmpRegName( res->Reg ), "0", NULL );
    }
    res->Type = doVarType(T_BOOL);
    return res;
}

struct InstrSeq*
doPrintBool( struct ExprRes* Expr )
{
    doDecArg(Expr);
    return doFuncInstrs(doCall("INTERNALPrintBool"));
}

struct ExprRes*
doNot( struct ExprRes* Expr )
{
    if ( Expr->Type->Type != T_BOOL || Expr->Type->isRef)
    {
        typeMismatch();
    }

    int reg_one = AvailTmpReg();
    struct InstrSeq* instrs;
    instrs = GenInstr( NULL, "addi", TmpRegName( reg_one ), "$0", "1" );
    AppendSeq( instrs, GenInstr( NULL, "xor",
                           TmpRegName( Expr->Reg ),
                           TmpRegName( reg_one ),
                           TmpRegName( Expr->Reg ) ) );
    AppendSeq( Expr->Instrs, instrs );
    ReleaseTmpReg( reg_one );
    return Expr;
}

struct ExprRes*
doBoolOp( struct ExprRes* Res1, struct ExprRes* Res2, int op )
{
    if ( Res1->Type->Type != T_BOOL || Res2->Type->Type != T_BOOL 
            || Res1->Type->isRef || Res2->Type->isRef)
    {
        typeMismatch();
    }

    char* opc;
    switch ( op )
    {
    case B_OR:
        opc = "or";
        break;
    case B_AND:
        opc = "and";
        break;
    }

    struct InstrSeq* instrs;
    instrs = GenInstr( NULL, opc,
        TmpRegName( Res1->Reg ),
        TmpRegName( Res1->Reg ),
        TmpRegName( Res2->Reg ) );
    AppendSeq( Res1->Instrs, Res2->Instrs );
    AppendSeq( Res1->Instrs, instrs );
    ReleaseTmpReg( Res2->Reg );
    free( Res2->Type );
    free( Res2 );
    return Res1;
}
