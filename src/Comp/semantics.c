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
extern struct StrLitList *strList;

/* Semantics support routines */

static void
typeMismatch()
{
    WriteIndicator( GetCurrentColumn() );
    WriteMessage( "Type Mismatch" );
    exit( 1 );
}

void doDeclare( char* name, int type )
{
    struct SymEntry* ent;
    EnterName( table, name, &ent );
    SetAttr( ent, (void*)type );
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

struct ExprRes *
doStrLit( char *str )
{
    struct ExprRes *res;
    struct StrLitList *strEnt;
    strEnt = malloc(sizeof(struct StrLitList));
    strEnt->label = GenLabel();
    strEnt->val = strdup(str);
    strEnt->next = strList;
    strList = strEnt;
    
    res = (struct ExprRes *)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "la", 
            TmpRegName(res->Reg),
            strEnt->label, NULL );
    res->Type = T_STR;
    return res;
}

struct ExprRes*
doRval( char* name )
{
    struct ExprRes* res;
    struct SymEntry* ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared Variable" );
        exit( 1 );
    }
    res = (struct ExprRes*)malloc( sizeof( struct ExprRes ) );
    res->Reg = AvailTmpReg();
    res->Instrs = GenInstr( NULL, "lw", TmpRegName( res->Reg ), name, NULL );
    res->Type = (int)GetAttr( ent );
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

struct InstrSeq *doPrintSpaceHlp()
{
    struct InstrSeq *code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_sp", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq *doPrintLn()
{
    struct InstrSeq *code;
    code = GenInstr( NULL, "li", "$v0", "4", NULL );
    AppendSeq( code, GenInstr( NULL, "la", "$a0", "_nl", NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    return code;
}

struct InstrSeq *doPrintList(struct ExprRes *Res1, struct InstrSeq *instrs)
{
    struct InstrSeq *res;
    res = doPrint(Res1);
    AppendSeq(res, doPrintSpaceHlp());
    AppendSeq(res, instrs);
    return res;
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

struct InstrSeq *
doPrintStr( struct ExprRes *Expr )
{
    struct InstrSeq *code;
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "li", "$v0", "4", NULL ) );
    AppendSeq( code, GenInstr( NULL, "move", "$a0", TmpRegName( Expr->Reg ), NULL ) );
    AppendSeq( code, GenInstr( NULL, "syscall", NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    free( Expr );
    return code;
}

struct InstrSeq*
doPrint( struct ExprRes* Expr )
{
    if ( Expr->Type == T_BOOL )
        return doPrintBool( Expr );
    if ( Expr->Type == T_INT )
        return doPrintInt( Expr );
    if ( Expr->Type == T_STR )
        return doPrintStr( Expr );
    
    typeMismatch();
    return NULL;
}

struct InstrSeq *
doPrintSp( struct ExprRes *Expr )
{
    if (Expr->Type != T_INT)
    {
        typeMismatch();
    }

    struct InstrSeq *code;
    int reg_count = AvailTmpReg();
    int reg_one = AvailTmpReg();
    char *s_label = GenLabel();
    char *e_label = GenLabel();
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "addi", 
                TmpRegName( reg_one ), 
                "$0", "1" ) );
    AppendSeq( code, GenInstr( NULL, "blt", 
                TmpRegName( Expr->Reg ),
                TmpRegName( reg_one ),
                e_label ) );
    AppendSeq( code, GenInstr( NULL, "move",
                TmpRegName(reg_count),
                "$0", NULL) );
    AppendSeq( code, GenInstr( s_label, NULL, NULL, NULL, NULL ) );
    AppendSeq( code, doPrintSpaceHlp() );
    AppendSeq( code, GenInstr( NULL, "add",
                TmpRegName( reg_count ),
                TmpRegName( reg_count ),
                TmpRegName( reg_one ) ) );
    AppendSeq( code, GenInstr( NULL, "blt",
                TmpRegName( reg_count ),
                TmpRegName( Expr->Reg ),
                s_label ) );
    AppendSeq( code, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg( Expr->Reg );
    ReleaseTmpReg( reg_count );
    ReleaseTmpReg( reg_one );
    free (Expr);
    free(s_label);
    free(e_label);
    return code;
}

struct InstrSeq*
doAssign( char* name, struct ExprRes* Expr )
{
    struct InstrSeq* code;
    struct SymEntry* ent = FindName( table, name );
    if ( !ent )
    {
        WriteIndicator( GetCurrentColumn() );
        WriteMessage( "Undeclared variable" );
        exit( 1 );
    }
    if ( (int)GetAttr( ent ) != Expr->Type )
    {
        typeMismatch();
    }
    code = Expr->Instrs;
    AppendSeq( code, GenInstr( NULL, "sw", TmpRegName( Expr->Reg ), name, NULL ) );
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

struct InstrSeq *doWhile( struct ExprRes *Expr, struct InstrSeq *code )
{
    if( Expr->Type != T_BOOL )
    {
        typeMismatch();
    }

    char *s_label = GenLabel();
    char *e_label = GenLabel();
    struct InstrSeq *nCode;

    nCode = GenInstr( s_label, NULL, NULL, NULL, NULL );
    AppendSeq( nCode, Expr->Instrs );
    AppendSeq( nCode, GenInstr( NULL, "beq",
                TmpRegName(Expr->Reg),
                "$0",
                e_label ) );
    AppendSeq( nCode, code );
    AppendSeq( nCode, GenInstr( NULL, "b", s_label, NULL, NULL ) );
    AppendSeq( nCode, GenInstr( e_label, NULL, NULL, NULL, NULL ) );
    ReleaseTmpReg(Expr->Reg);
    free( s_label );
    free( e_label );
    free( Expr );
    return nCode;
}

void Finish( struct InstrSeq* Code )
{
    struct InstrSeq* code;
    struct SymEntry* entry;
    struct Attr* attr;
    struct StrLitList *strEnt;

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
    AppendSeq( code, GenInstr( "_sp", ".asciiz", "\" \"", NULL, NULL ) );

    entry = FirstEntry( table );
    while ( entry )
    {
        AppendSeq( code, GenInstr( (char*)GetName( entry ), ".word", "0", NULL, NULL ) );
        entry = NextEntry( table, entry );
    }

    strEnt = strList;
    while ( strEnt )
    {
        AppendSeq( code, GenInstr( strEnt->label, ".asciiz", strEnt->val, NULL, NULL ) );
        strEnt = strEnt->next;
    }

    WriteSeq( code );

    return;
}

