#include "semantics.h"

struct ExprRes *
doIntToFloat( struct ExprRes *Expr )
{
    if(Expr->Type->Type != T_INT || Expr->Type->isRef)
    {
        typeMismatch();
    }

    AppendSeq( Expr->Instrs, GenInstr( NULL, "mtc1",
                                 TmpRegName( Expr->Reg ),
                                 "$f0",
                                 NULL ) );
    AppendSeq(Expr->Instrs, GenInstr(NULL, "cvt.s.w",
                "$f0", "$f0", NULL ));
    AppendSeq(Expr->Instrs, GenInstr(NULL, "mfc1",
                TmpRegName(Expr->Reg),
                "$f0", NULL ) );
    Expr->Type->Type = T_FLOAT;
    return Expr;
}

struct ExprRes *
doFloatToInt( struct ExprRes *Expr )
{
    if(Expr->Type->Type != T_FLOAT || Expr->Type->isRef)
    {
        typeMismatch();
    }

    AppendSeq( Expr->Instrs, GenInstr( NULL, "mtc1",
                                 TmpRegName( Expr->Reg ),
                                 "$f0",
                                 NULL ) );
    AppendSeq( Expr->Instrs, GenInstr( NULL, "cvt.w.s",
                "$f0", "$f0", NULL ) );
    AppendSeq( Expr->Instrs, GenInstr( NULL, "mfc1",
                TmpRegName(Expr->Reg),
                "$f0", NULL) );
    Expr->Type->Type = T_INT;
    return Expr;
}

struct ExprRes* 
doFloatLit( float val )
{
    char buf[ 1024 ];
    snprintf( buf, 1024, "%f", val );
    struct ExprRes* res;
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "li.s", "$f0", buf, NULL );
    AppendSeq( res->Instrs, GenInstr( NULL, "mfc1",
                                TmpRegName( res->Reg ),
                                "$f0",
                                NULL ) );
    res->Type = doVarType( T_FLOAT );
    return res;
}

struct ExprRes*
doReadFloat( struct IdAddr* addr )
{
    struct ExprRes* res = malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Type = malloc( sizeof( struct VarType ) );
    memcpy( res->Type, addr->Addr->Type, sizeof( struct VarType ) );
    res->Instrs = GenInstr( NULL, "li", "$v0", "6", NULL );
    AppendSeq( res->Instrs, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( res->Instrs, GenInstr( NULL, "mfc1",
                TmpRegName( res->Reg ),
                "$f0", NULL ) );
    return res;
}

struct ExprRes* 
doArithFloat( struct ExprRes* Res1, struct ExprRes* Res2, char op )
{
    int reg = AvailTmpReg();
    char* opc;

    switch ( op )
    {
    case '+':
        opc = "add.s";
        break;
    case '-':
        opc = "sub.s";
        break;
    case '*':
        opc = "mul.s";
        break;
    case '/':
        opc = "div.s";
        break;
    default:
        typeMismatch();
    }

    AppendSeq( Res1->Instrs, Res2->Instrs );
    AppendSeq( Res1->Instrs, GenInstr( NULL, "mtc1",
                                 TmpRegName( Res1->Reg ),
                                 "$f0",
                                 NULL ) );
    AppendSeq( Res1->Instrs, GenInstr( NULL, "mtc1",
                                 TmpRegName( Res2->Reg ),
                                 "$f2",
                                 NULL ) );
    AppendSeq( Res1->Instrs, GenInstr( NULL, opc,
                                 "$f0",
                                 "$f0",
                                 "$f2" ) );
    AppendSeq( Res1->Instrs, GenInstr( NULL, "mfc1",
                                 TmpRegName( reg ),
                                 "$f0", NULL ) );
    ReleaseTmpReg( Res1->Reg );
    ReleaseTmpReg( Res2->Reg );
    Res1->Reg = reg;
    free( Res2->Type );
    free( Res2 );
    return Res1;
}

struct ExprRes* 
doNegateFloat( struct ExprRes* Expr )
{
    doDecArg(Expr);
    return doCall("INTERNALNegateFloat");
}

struct InstrSeq* 
doPrintFloat( struct ExprRes* Expr )
{
    struct InstrSeq* code;
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "2", NULL ) );
    AppendSeq( code, GenInstr( NULL, "mtc1", TmpRegName( Expr->Reg ), "$f12", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr->Type );
    free( Expr );
    return code;
}

struct ExprRes* 
doCompFloat( struct ExprRes* Res1, struct ExprRes* Res2, int op )
{
    struct InstrSeq* instrs;
    char* f_label = GenLabel();
    char* t_label = GenLabel();
    char* e_label = GenLabel();
    int reg_result = AvailTmpReg();

    instrs = GenInstr( NULL, "mtc1",
        TmpRegName( Res1->Reg ),
        "$f0",
        NULL );
    AppendSeq( instrs, GenInstr( NULL, "mtc1",
                           TmpRegName( Res2->Reg ),
                           "$f2",
                           NULL ) );
    switch ( op )
    {
    case B_LT:
        AppendSeq(instrs, GenInstr( NULL, "c.lt.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1t", t_label, NULL, NULL));
        break;
    case B_LTE:
        AppendSeq(instrs, GenInstr( NULL, "c.le.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1t", t_label, NULL, NULL));
        break;
    case B_GT:
        AppendSeq(instrs, GenInstr( NULL, "c.le.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1f", t_label, NULL, NULL));
        break;
    case B_GTE:
        AppendSeq(instrs, GenInstr( NULL, "c.lt.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1f", t_label, NULL, NULL));
        break;
    case B_EQ:
        AppendSeq(instrs, GenInstr( NULL, "c.eq.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1t", t_label, NULL, NULL));
        break;
    case B_NE:
        AppendSeq(instrs, GenInstr( NULL, "c.eq.s","$f0", "$f2", NULL));
        AppendSeq(instrs, GenInstr( NULL, "bc1f", t_label, NULL, NULL));
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
    free( Res2->Type );
    free( Res2 );
    free( f_label );
    free( t_label );
    free( e_label );
    Res1->Reg = reg_result;
    Res1->Type = doVarType( T_BOOL );
    return Res1;
}

struct ExprRes*
doPowFloat( struct ExprRes* base, struct ExprRes* pow )
{
    doDecArg(base);
    doDecArg(pow);
    return doCall("INTERNALPowFloat");
}
