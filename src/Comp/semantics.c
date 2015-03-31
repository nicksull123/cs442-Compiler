/* Semantics.c
   Support and semantic action routines.
*/

#include <strings.h>
#include <stdlib.h>

#include "codegen.h"
#include "semantics.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern struct SymTab* table;

/* Semantics support routines */

void
doDeclare( char *name, int type )
{
    struct SymEntry *ent;
    EnterName(table, name, &ent);
    SetAttr(ent, (void *)type);
}

struct ExprRes*
doIntLit( char* digits )
{
    struct ExprRes* res;
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "li", TmpRegName( res->Reg ), digits, NULL );
    res->Type = T_INT;
    return res;
}

struct ExprRes *
doBoolLit( int b )
{
    struct ExprRes *res;
    res = (struct ExprRes *)malloc( sizeof( struct ExprRes ) );
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
doRval( char* name )
{
    struct ExprRes* res;
    struct SymEntry *ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit(1);
    }
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "lw", TmpRegName( res->Reg ), name, NULL );
    res->Type = (int)GetAttr(ent);
    return res;
}

struct ExprRes*
doNegate( struct ExprRes* Expr )
{
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

struct ExprRes *
doPow( struct ExprRes *base, struct ExprRes *pow)
{
    struct InstrSeq *instrs;
    int reg_pow = AvailTmpReg();
    int reg_cur = AvailTmpReg();
    char *s_label = GenLabel();
    char *e_label = GenLabel();

    instrs = GenInstr( NULL, "move", TmpRegName( reg_pow ), "$0", NULL );
    AppendSeq(instrs, GenInstr( NULL, "addi", TmpRegName( reg_cur ), "$0", "1" ) );
    AppendSeq(instrs, GenInstr( NULL, "beq", TmpRegName( pow->Reg ), "$0", e_label) );
    AppendSeq(instrs, GenInstr( s_label, NULL, NULL, NULL, NULL ) );
    AppendSeq(instrs, GenInstr( NULL, "mul", TmpRegName( reg_cur ),
                        TmpRegName( reg_cur ),
                        TmpRegName( base->Reg) ) );
    AppendSeq(instrs, GenInstr( NULL, "addi", TmpRegName( reg_pow ),
                        TmpRegName( reg_pow ), 
                        "1") );
    AppendSeq(instrs, GenInstr( NULL, "blt", TmpRegName( reg_pow ), 
                TmpRegName(pow->Reg), s_label ) );
    AppendSeq(instrs, GenInstr( e_label, NULL, NULL, NULL, NULL ) );

    AppendSeq(base->Instrs, pow->Instrs);
    AppendSeq(base->Instrs, instrs);
    ReleaseTmpReg(base->Reg);
    ReleaseTmpReg(pow->Reg);
    ReleaseTmpReg(reg_pow);
    base->Reg = reg_cur;
    free( s_label );
    free( e_label );
    free( pow );
    return base;
}

struct ExprRes*
doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op )
{
    int reg = AvailTmpReg();
    char *opc;

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

    AppendSeq( code, GenInstr( NULL, "li", "$v0", "4", NULL ) );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_nl", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );

    ReleaseTmpReg( Expr->Reg );
    free( Expr );

    return code;
}

struct InstrSeq *
doPrintBool( struct ExprRes *Expr )
{
    struct InstrSeq *code;
    char *e_label = GenLabel();
    char *f_label = GenLabel();
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
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_nl", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    free( e_label );
    free( f_label );
    return code;
}

struct InstrSeq *
doPrint( struct ExprRes *Expr )
{
    if (Expr->Type == T_BOOL)
        return doPrintBool(Expr);
    return doPrintInt(Expr);
}

struct InstrSeq*
doAssign( char* name, struct ExprRes *Expr )
{
    struct InstrSeq* code;
    struct SymEntry *ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit(1);
    }
    if ((int)GetAttr(ent) != Expr->Type)
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Type mismatch" );
        exit(1);
    }
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "sw", TmpRegName( Expr->Reg ), name, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}

struct ExprRes *
doBExpr( struct ExprRes *Res1, struct ExprRes *Res2, int op )
{
    struct InstrSeq *instrs;
    int reg_result = AvailTmpReg();

    puts("FUCK FUCK FUCK");
    AppendSeq( Res1->Instrs, Res2->Instrs );
    AppendSeq( Res1->Instrs, instrs );
    ReleaseTmpReg( Res1->Reg );
    ReleaseTmpReg( Res2->Reg );
    free( Res2 );
    Res1->Reg = reg_result;
    Res1->Type = T_BOOL;
    return Res1;
}

/*struct BExprRes*
doBExpr( struct ExprRes* Res1, struct ExprRes* Res2, int op )
{
    struct BExprRes* bRes;
    AppendSeq( Res1->Instrs, Res2->Instrs );
    bRes = (struct BExprRes*)malloc( sizeof( struct BExprRes ) );
    bRes->Label = GenLabel();
    AppendSeq( Res1->Instrs, GenInstr( NULL, "bne", TmpRegName( Res1->Reg ), TmpRegName( Res2->Reg ), bRes->Label ) );
    bRes->Instrs = Res1->Instrs;
    ReleaseTmpReg( Res1->Reg );
    ReleaseTmpReg( Res2->Reg );
    free( Res1 );
    free( Res2 );
    return bRes;
}

struct InstrSeq*
doIf( struct BExprRes* bRes, struct InstrSeq* seq )
{
    struct InstrSeq* seq2;
    seq2 = AppendSeq( bRes->Instrs, seq );
    AppendSeq( seq2, GenInstr( bRes->Label, NULL, NULL, NULL, NULL ) );
    free( bRes );
    return seq2;
}

*/

/*

extern struct InstrSeq * doIf(struct ExprRes *res1, struct ExprRes *res2, struct InstrSeq * seq) {
    struct InstrSeq *seq2;
    char * label;
    label = GenLabel();
    AppendSeq(res1->Instrs, res2->Instrs);
    AppendSeq(res1->Instrs, GenInstr(NULL, "bne", TmpRegName(res1->Reg), TmpRegName(res2->Reg), label));
    seq2 = AppendSeq(res1->Instrs, seq);
    AppendSeq(seq2, GenInstr(label, NULL, NULL, NULL, NULL));
    ReleaseTmpReg(res1->Reg);
    ReleaseTmpReg(res2->Reg);
    free(res1);
    free(res2);
    return seq2;
}
*/

void Finish( struct InstrSeq* Code )
{
    struct InstrSeq* code;
    struct SymEntry* entry;
    struct Attr* attr;

    code = GenInstr( NULL, ".text", NULL, NULL, NULL );
    AppendSeq( code, GenInstr( NULL, ".globl", "main", NULL, NULL ) );
    AppendSeq( code, GenInstr( "main", NULL, NULL, NULL, NULL ) );
    AppendSeq( code, Code );
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "10", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".data", NULL, NULL, NULL ) );
    AppendSeq( code, GenInstr( NULL, ".align", "4", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_nl", ".asciiz", "\"\\n\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_true", ".asciiz", "\"true\"", NULL, NULL ) );
    AppendSeq( code, GenInstr( "_false", ".asciiz", "\"false\"", NULL, NULL ) );

    entry = FirstEntry( table );
    while ( entry )
    {
        AppendSeq( code, GenInstr( (char*)GetName( entry ), ".word", "0", NULL, NULL ) );
        entry = NextEntry( table, entry );
    }

    WriteSeq( code );

    return;
}

