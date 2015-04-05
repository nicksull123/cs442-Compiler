#include "semantics.h"

struct ExprRes *
doDeRef(char *name)
{
    struct ExprRes *res = doRval(name);
    if(!res->Type->isRef)
    {
        typeMismatch();
    }
    res->Type->isRef = 0;
    AppendSeq(res->Instrs, GenInstr(NULL, "lw",
                TmpRegName(res->Reg),
                RegOff(0, TmpRegName(res->Reg)),
                NULL));
    return res;
}

struct ExprRes *
doAddr(char *name)
{
    struct VarType *vType = doFindVar(name); 
    if(vType->isRef)
    {
        typeMismatch();
    }
    struct ExprRes *res = malloc(sizeof(struct ExprRes));
    res->Reg = AvailTmpReg();
    res->Type = malloc(sizeof(struct VarType));
    memcpy(res->Type, vType, sizeof(struct VarType));
    res->Type->isRef = 1;
    if(vType->Loc == V_GBL)
    {
        res->Instrs = GenInstr(NULL, "la", 
                TmpRegName(res->Reg),
                name, NULL);
    }
    else
    {
        res->Instrs = GenInstr(NULL, "move",
                TmpRegName(res->Reg),
                "$sp", NULL);
        AppendSeq(res->Instrs, GenInstr(NULL, "addi",
                    TmpRegName(res->Reg),
                    TmpRegName(res->Reg),
                    Imm(vType->SPos)));
    }
    return res;
}
