/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

#pragma once

#include <strings.h>
#include <stdlib.h>
#include "codegen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern struct SymTab* table;
extern struct SymTab *funcTab;
extern struct StrLitList *strList;

/* Semantic Defines */
#define T_BOOL 0
#define T_BOOL_ARR 1
#define T_INT 2
#define T_INT_ARR 3
#define T_STR 4

#define B_LT 0
#define B_LTE 1
#define B_GT 2
#define B_GTE 3
#define B_NE 4
#define B_EQ 5
#define B_OR 6
#define B_AND 7

#define B_TRUE 1
#define B_FALSE 0

/* Semantic Records */
struct VarType
{
    int Type;
    int Size;
};

struct FuncType
{
    int Type;
};

struct IdList
{
    struct SymEntry* TheEntry;
    struct IdList* Next;
};

struct ExprRes
{
    int Reg;
    int Type;
    struct InstrSeq* Instrs;
};

struct ExprResList
{
    struct ExprRes* Expr;
    struct ExprResList* Next;
};

struct StrLitList
{
    char *label;
    char *val;
    struct StrLitList *next;
};

/* Semantics Actions */
void doDeclare(char *name, int type);
void typeMismatch();
struct ExprRes* doRval( char* name );
struct InstrSeq* doAssign( char* name, struct ExprRes *Expr );
struct InstrSeq *doPrintList(struct ExprRes *Res1, struct InstrSeq *instrs2);
struct InstrSeq* doPrint( struct ExprRes* Expr );
struct InstrSeq *doPrintLn();
struct InstrSeq *doPrintSp( struct ExprRes *Expr );
struct InstrSeq *doRead( char *var );
struct InstrSeq *doReadList( char *var, struct InstrSeq *code );
void Finish( struct InstrSeq* Code );

/* Control Semantics Actions */
struct InstrSeq *doWhile( struct ExprRes *Expr, struct InstrSeq *code );
struct InstrSeq *doIfElse( struct ExprRes *Expr, struct InstrSeq *iCode, struct InstrSeq *eCode );
struct InstrSeq *doIf( struct ExprRes *Expr, struct InstrSeq *code );

/* Arrays Semantics Actions */
void doDeclareArr(char *name, int type, int size);
struct InstrSeq *doAssignArr( char *name, struct ExprRes *Expr, struct ExprRes *Pos);
struct ExprRes *doArrVal( char *name, struct ExprRes *Pos );

/* Functions Semantics Actions */
struct InstrSeq *doReturn( struct ExprRes *Expr );
struct ExprRes *doCall( char *name);
struct InstrSeq *doDecFunc( char *name, struct InstrSeq *code, int type );

/* Bool Semantics Actions */
struct ExprRes *doBoolLit( int b );
struct ExprRes *doNot( struct ExprRes *Expr );
struct ExprRes *doBoolOp( struct ExprRes *Res1, struct ExprRes *Res2, int op );
struct InstrSeq* doPrintBool( struct ExprRes* Expr );

/* Int Semantics Actions */
struct ExprRes* doComp( struct ExprRes* Res1, struct ExprRes* Res2, int op );
struct ExprRes* doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op );
struct ExprRes *doPow( struct ExprRes *base, struct ExprRes *pow );
struct ExprRes* doIntLit( char* digits );
struct ExprRes *doNegate( struct ExprRes *Expr );
struct InstrSeq* doPrintInt( struct ExprRes* Expr );

/* Str Semantics Actions */
struct InstrSeq *doPrintStr( struct ExprRes *Expr );
struct ExprRes *doStrLit( char *str );
