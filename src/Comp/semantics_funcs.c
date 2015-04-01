#include "semantics.h"

struct InstrSeq *
doReturn( struct ExprRes *Expr )
{
    struct InstrSeq *code = Expr->Instrs;
    AppendSeq(code, GenInstr(NULL, "move",
                "$v0",
                TmpRegName(Expr->Reg), NULL));
    AppendSeq(code, GenInstr(NULL, "jr", "$ra", NULL, NULL));
    ReleaseTmpReg(Expr->Reg);
    free( Expr );
    return code;
}

struct ExprRes *
doCall( char *name )
{
    char buf[1024];
    snprintf(buf, 1024, "_%s", name);
    struct SymEntry *entry = FindName( funcTab, buf );
    if( !entry )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Function not declared" );
        exit( 1 );
    }
    struct ExprRes *ret = malloc(sizeof(struct ExprRes));
    struct FuncType *fType = (struct FuncType *)GetAttr(entry);
    ret->Type = fType->Type;
    ret->Reg = AvailTmpReg();
    ret->Instrs = SaveSeq();
    AppendSeq(ret->Instrs, GenInstr(NULL, "jal", buf, NULL, NULL));;
    AppendSeq(ret->Instrs, RestoreSeq());
    AppendSeq(ret->Instrs, GenInstr(NULL, "move",
                TmpRegName(ret->Reg),
                "$v0", NULL));
    return ret;
}

struct InstrSeq *
doDecFunc( char *name, struct InstrSeq *code, int type )
{
    char buf[1024];
    snprintf(buf, 1024, "_%s", name);
    struct InstrSeq *nCode;
    struct SymEntry *entry;
    struct FuncType *fType;
    if( FindName(funcTab, buf) )
    {
        WriteIndicator(GetCurrentColumn());
        WriteMessage("Function Redefinition Illegal");
        exit( 1 );
    }
    EnterName(funcTab, buf, &entry);
    fType = malloc(sizeof(struct FuncType));
    fType->Type = type;
    SetAttr(entry, (void *)fType);
    nCode = GenInstr(buf, NULL, NULL, NULL, NULL);
    AppendSeq(nCode, code);
    AppendSeq(nCode, GenInstr(NULL, "jr", "$ra", NULL, NULL) );
    return nCode;
}
