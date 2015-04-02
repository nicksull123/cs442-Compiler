#include "semantics.h"

struct ExprRes*
doStrLit( char* str )
{
    struct ExprRes* res;
    struct StrLitList* strEnt;
    strEnt = malloc( sizeof( struct StrLitList ) );
    strEnt->label = GenLabel();
    strEnt->val = strdup( str );
    strEnt->next = strList;
    strList = strEnt;

    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "la",
        TmpRegName( res->Reg ),
        strEnt->label, NULL );
    res->Type = T_STR;
    return res;
}

struct InstrSeq*
doPrintStr( struct ExprRes* Expr )
{
    struct InstrSeq* code;
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "4", NULL ) );
    AppendSeq( code, GenInstr( NULL, "move", "$a0", TmpRegName( Expr->Reg ), NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}
