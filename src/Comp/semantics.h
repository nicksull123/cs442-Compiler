/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

#pragma once

#include <strings.h>
#include <stdlib.h>
#include "codegen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern struct SymTab* curTab;
extern struct SymTab* funcTab;
extern struct TabList* tabList;
extern struct ArgList *argList;
extern struct StrLitList* strList;
extern int paramPos;
extern int argPos;
extern int sPos;

/* Semantic Defines */
#define T_BOOL 0
#define T_BOOL_ARR 1
#define T_INT 2
#define T_INT_ARR 3
#define T_STR 4
#define T_ANY 5

#define V_GBL 0
#define V_LOC 1

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
    int Loc;
    int SPos;
    int Arg;
    int ArgPos;
};

struct FuncType
{
    int Type;
    int VarRsrv;
    struct SymTab *Tab;
};

struct TabList
{
    struct SymTab* Tab;
    struct TabList* Next;
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
    char* label;
    char* val;
    struct StrLitList* next;
};

struct ArgList
{
    int ArgPos;
    struct ExprRes *Res;
    struct ArgList *Next;
};

/* Semantics Actions */
void doDeclare( char* name, int type, int arg );
void doPushDecs();
void doPopDecs();
void typeMismatch();
struct VarType* doFindVar( char* name );
struct ExprRes* doRval( char* name );
struct InstrSeq* doAssign( char* name, struct ExprRes* Expr, int SZOff );
struct InstrSeq* doPrintList( struct ExprRes* Res1, struct InstrSeq* instrs2 );
struct InstrSeq* doPrint( struct ExprRes* Expr );
struct InstrSeq* doPrintLn();
struct InstrSeq* doPrintSp( struct ExprRes* Expr );
struct InstrSeq* doRead( char* var );
void Finish( struct InstrSeq* Code );

/* Control Semantics Actions */
struct InstrSeq* doWhile( struct ExprRes* Expr, struct InstrSeq* code );
struct InstrSeq* doIfElse( struct ExprRes* Expr, struct InstrSeq* iCode, struct InstrSeq* eCode );
struct InstrSeq* doIf( struct ExprRes* Expr, struct InstrSeq* code );

/* Arrays Semantics Actions */
void doDeclareArr( char* name, int type, int size );
struct InstrSeq* doAssignArr( char* name, struct ExprRes* Expr, struct ExprRes* Pos );
struct ExprRes* doArrVal( char* name, struct ExprRes* Pos );
struct InstrSeq* doReadArr( char* name, struct ExprRes* Pos );

/* Functions Semantics Actions */
struct InstrSeq* doReturn( struct ExprRes* Expr );
struct ExprRes* doCall( char* name );
void doFuncInit( char *name, int type );
struct InstrSeq* doDecFunc( char* name, struct InstrSeq* code );
struct InstrSeq* doFuncInstrs( struct ExprRes* res );
void doDecArg( struct ExprRes *res );

/* Bool Semantics Actions */
struct ExprRes* doBoolLit( int b );
struct ExprRes* doNot( struct ExprRes* Expr );
struct ExprRes* doBoolOp( struct ExprRes* Res1, struct ExprRes* Res2, int op );
struct InstrSeq* doPrintBool( struct ExprRes* Expr );

/* Int Semantics Actions */
struct ExprRes* doComp( struct ExprRes* Res1, struct ExprRes* Res2, int op );
struct ExprRes* doArith( struct ExprRes* Res1, struct ExprRes* Res2, char op );
struct ExprRes* doPow( struct ExprRes* base, struct ExprRes* pow );
struct ExprRes* doIntLit( int val );
struct ExprRes* doNegate( struct ExprRes* Expr );
struct InstrSeq* doPrintInt( struct ExprRes* Expr );

/* Str Semantics Actions */
struct InstrSeq* doPrintStr( struct ExprRes* Expr );
struct ExprRes* doStrLit( char* str );
