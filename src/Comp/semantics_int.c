#include "semantics.h"

struct ExprRes*
doIntLit( int val )
{
    struct ExprRes* res;
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "li", TmpRegName( res->Reg ), Imm(val), NULL );
    res->Type = T_INT;
    return res;
}

struct ExprRes*
doPow( struct ExprRes* base, struct ExprRes* pow )
{
    if ( base->Type != T_INT || pow->Type != T_INT )
    {
        typeMismatch();
    }
    struct InstrSeq* instrs;
    int reg_pow = AvailTmpReg();
    int reg_cur = AvailTmpReg();
    char* s_label = GenLabel();
    char* e_label = GenLabel();

    instrs = GenInstr( NULL, "move", TmpRegName( reg_pow ), "$0", NULL );
    AppendSeq( instrs, GenInstr( NULL, "addi", TmpRegName( reg_cur ), "$0", "1" ) );
    AppendSeq( instrs, GenInstr( NULL, "beq", TmpRegName( pow->Reg ), "$0", e_label ) );
    AppendSeq( instrs, GenInstr( s_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( instrs, GenInstr( NULL, "mul", TmpRegName( reg_cur ),
                           TmpRegName( reg_cur ),
                           TmpRegName( base->Reg ) ) );
    AppendSeq( instrs, GenInstr( NULL, "addi", TmpRegName( reg_pow ),
                           TmpRegName( reg_pow ),
                           "1" ) );
    AppendSeq( instrs, GenInstr( NULL, "blt", TmpRegName( reg_pow ),
                           TmpRegName( pow->Reg ), s_label ) );
    AppendSeq( instrs, GenInstr( e_label, NULL, NULL, NULL, NULL ) );

    AppendSeq( base->Instrs, pow->Instrs );
    AppendSeq( base->Instrs, instrs );
    ReleaseTmpReg( base->Reg );
    ReleaseTmpReg( pow->Reg );
    ReleaseTmpReg( reg_pow );
    base->Reg = reg_cur;
    free( s_label );
    free( e_label );
    free( pow );
    return base;
}

struct ExprRes*
doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op )
{
    if ( Res1->Type != T_INT || Res2->Type != T_INT )
    {
        typeMismatch();
    }
    int reg = AvailTmpReg();
    char* opc;

    switch ( op )
    {
    case '+':
        opc = "add";
        break;
    case '-':
        opc = "sub";
        break;
    case '*':
        opc = "mul";
        break;
    case '/':
        opc = "div";
        break;
    case '%':
        opc = "rem";
        break;
    }

    AppendSeq( Res1->Instrs, Res2->Instrs );
    AppendSeq( Res1->Instrs, GenInstr( NULL, opc,
                                 TmpRegName( reg ),
                                 TmpRegName( Res1->Reg ),
                                 TmpRegName( Res2->Reg ) ) );
    ReleaseTmpReg( Res1->Reg );
    ReleaseTmpReg( Res2->Reg );
    Res1->Reg = reg;
    free( Res2 );
    return Res1;
}

struct InstrSeq*
doPrintInt( struct ExprRes* Expr )
{
    struct InstrSeq* code;
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "1", NULL ) );
    AppendSeq( code, GenInstr( NULL, "move", "$a0", TmpRegName( Expr->Reg ), NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}

struct ExprRes*
doComp( struct ExprRes* Res1, struct ExprRes* Res2, int op )
{
    if ( Res1->Type != T_INT || Res2->Type != T_INT )
    {
        typeMismatch();
    }

    struct InstrSeq* instrs;
    char* f_label = GenLabel();
    char* t_label = GenLabel();
    char* e_label = GenLabel();
    int reg_result = AvailTmpReg();

    switch ( op )
    {
    case B_LT:
        instrs = GenInstr( NULL, "blt",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        break;
    case B_LTE:
        instrs = GenInstr( NULL, "blt",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        AppendSeq( instrs, GenInstr( NULL, "beq",
                               TmpRegName( Res1->Reg ),
                               TmpRegName( Res2->Reg ),
                               t_label ) );
        break;
    case B_GT:
        instrs = GenInstr( NULL, "bgt",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        break;
    case B_GTE:
        instrs = GenInstr( NULL, "bgt",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        AppendSeq( instrs, GenInstr( NULL, "beq",
                               TmpRegName( Res1->Reg ),
                               TmpRegName( Res2->Reg ),
                               t_label ) );
        break;
    case B_EQ:
        instrs = GenInstr( NULL, "beq",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        break;
    case B_NE:
        instrs = GenInstr( NULL, "bne",
            TmpRegName( Res1->Reg ),
            TmpRegName( Res2->Reg ),
            t_label );
        break;
    }

    AppendSeq( instrs, GenInstr( NULL, "b", f_label, NULL, NULL ) );
    AppendSeq( instrs, GenInstr( f_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( instrs, GenInstr( NULL, "li", TmpRegName( reg_result ), "0", NULL ) );
    AppendSeq( instrs, GenInstr( NULL, "b", e_label, NULL, NULL ) );
    AppendSeq( instrs, GenInstr( t_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( instrs, GenInstr( NULL, "li", TmpRegName( reg_result ), "1", NULL ) );
    AppendSeq( instrs, GenInstr( e_label, NULL, NULL, NULL, NULL ) );

    AppendSeq( Res1->Instrs, Res2->Instrs );
    AppendSeq( Res1->Instrs, instrs );
    ReleaseTmpReg( Res1->Reg );
    ReleaseTmpReg( Res2->Reg );
    free( Res2 );
    free( f_label );
    free( t_label );
    free( e_label );
    Res1->Reg = reg_result;
    Res1->Type = T_BOOL;
    return Res1;
}
