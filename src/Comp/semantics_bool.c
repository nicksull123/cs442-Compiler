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
    res->Type = T_BOOL;
    return res;
}

struct ExprRes*
doNegate( struct ExprRes* Expr )
{
    if ( Expr->Type != T_INT )
    {
        typeMismatch();
    }
    struct InstrSeq* inst;
    int reg = AvailTmpReg();
    inst = GenInstr( NULL, "sub",
        TmpRegName( reg ),
        "$0",
        TmpRegName( Expr->Reg ) );
    AppendSeq( Expr->Instrs, inst );
    ReleaseTmpReg( Expr->Reg );
    Expr->Reg = reg;
    return Expr;
}

struct InstrSeq*
doPrintBool( struct ExprRes* Expr )
{
    struct InstrSeq* code;
    char* e_label = GenLabel();
    char* f_label = GenLabel();
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "4", NULL ) );
    AppendSeq( code, GenInstr( NULL, "beq", TmpRegName( Expr->Reg ), "$0", f_label ) );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_true", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, "b", e_label, NULL, NULL ) );
    AppendSeq( code, GenInstr( f_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_false", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    free( e_label );
    free( f_label );
    return code;
}

struct ExprRes*
doNot( struct ExprRes* Expr )
{
    if ( Expr->Type != T_BOOL )
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
    if ( Res1->Type != T_BOOL || Res2->Type != T_BOOL )
    {
        typeMismatch();
    }
    
    char *opc;
    switch (op)
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
    free( Res2 );
    return Res1;
}
