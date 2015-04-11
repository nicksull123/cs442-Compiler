#include "semantics.h"

struct InstrSeq *
doIf(struct ExprRes *Expr, struct InstrSeq *code)
{
    if (Expr->Type->Type != T_BOOL || Expr->Type->isRef)
    {
        typeMismatch();
    }

    struct InstrSeq *nCode = Expr->Instrs;
    char *end_label = GenLabel();
    AppendSeq(
        nCode, GenInstr(NULL, "beq", TmpRegName(Expr->Reg), "$0", end_label));
    AppendSeq(nCode, code);
    AppendSeq(nCode, GenInstr(end_label, NULL, NULL, NULL, NULL));
    ReleaseTmpReg(Expr->Reg);
    free(end_label);
    free(Expr->Type);
    free(Expr);
    return nCode;
}

struct InstrSeq *
doIfElse(struct ExprRes *Expr, struct InstrSeq *iCode, struct InstrSeq *eCode)
{
    if (Expr->Type->Type != T_BOOL || Expr->Type->isRef)
    {
        typeMismatch();
    }

    struct InstrSeq *nCode = Expr->Instrs;
    char *end_label = GenLabel();
    char *else_label = GenLabel();
    AppendSeq(
        nCode, GenInstr(NULL, "beq", TmpRegName(Expr->Reg), "$0", else_label));
    AppendSeq(nCode, iCode);
    AppendSeq(nCode, GenInstr(NULL, "b", end_label, NULL, NULL));
    AppendSeq(nCode, GenInstr(else_label, NULL, NULL, NULL, NULL));
    AppendSeq(nCode, eCode);
    AppendSeq(nCode, GenInstr(end_label, NULL, NULL, NULL, NULL));
    ReleaseTmpReg(Expr->Reg);
    free(end_label);
    free(else_label);
    free(Expr->Type);
    free(Expr);
    return nCode;
}

struct InstrSeq *
doWhile(struct ExprRes *Expr, struct InstrSeq *code)
{
    if (Expr->Type->Type != T_BOOL || Expr->Type->isRef)
    {
        typeMismatch();
    }

    char *s_label = GenLabel();
    char *e_label = GenLabel();
    struct InstrSeq *nCode;
    nCode = GenInstr(s_label, NULL, NULL, NULL, NULL);
    AppendSeq(nCode, Expr->Instrs);
    AppendSeq(
        nCode, GenInstr(NULL, "beq", TmpRegName(Expr->Reg), "$0", e_label));
    AppendSeq(nCode, code);
    AppendSeq(nCode, GenInstr(NULL, "b", s_label, NULL, NULL));
    AppendSeq(nCode, GenInstr(e_label, NULL, NULL, NULL, NULL));
    ReleaseTmpReg(Expr->Reg);
    free(s_label);
    free(e_label);
    free(Expr->Type);
    free(Expr);
    return nCode;
}

