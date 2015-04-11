/* Semantics.h
   The action and supporting routines for performing semantics processing.
*/

#pragma once

#include <strings.h>
#include <stdlib.h>
#include "codegen.h"
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"

extern struct SymTab *curTab;
extern struct SymTab *funcTab;
extern struct TabList *tabList;
extern struct ArgList *argList;
extern struct StrLitList *strList;
extern unsigned char libclite_cl[];
extern unsigned int libclite_cl_len;
extern int parseStdLib;
extern int paramPos;
extern int argPos;
extern int sPos;

/* Semantic Defines */
#define T_BOOL 0
#define T_INT 1
#define T_STR 2
#define T_FLOAT 3

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
    int isRef;
    int Size;
    int Loc;
    int SPos;
    int isArg;
    int ArgPos;
    char *Shim;
};

struct FuncType
{
    int Type;
    int VarRsrv;
    struct SymTab *Tab;
};

struct TabList
{
    struct SymTab *Tab;
    struct TabList *Next;
};

struct ExprRes
{
    int Reg;
    struct VarType *Type;
    struct InstrSeq *Instrs;
};

struct StrLitList
{
    char *label;
    char *val;
    struct StrLitList *next;
};

struct ArgList
{
    int ArgPos;
    struct ExprRes *Res;
    struct ArgList *Next;
};

struct IdAddr
{
    char *Name;
    struct ExprRes *Addr;
};

/* Semantics Actions */
int getLexInput();
void doDeclare(char *name, struct VarType *type, int arg, int size);
void doPushDecs();
void doPopDecs();
void typeMismatch();
struct VarType *doVarType(int type);
struct VarType *doFindVar(char *name);
struct ExprRes *doRval(struct IdAddr *addr);
struct InstrSeq *
doAssign(struct IdAddr *addr, struct ExprRes *Expr, int inverse);
struct InstrSeq *doPrintList(struct ExprRes *Res1, struct InstrSeq *instrs2);
struct InstrSeq *doPrint(struct ExprRes *Expr);
struct InstrSeq *doPrintLn();
struct InstrSeq *doPrintSp(struct ExprRes *Expr);
struct InstrSeq *doRead(struct IdAddr *addr);
struct ExprRes *doComp(struct ExprRes *Res1, struct ExprRes *Res2, int op);
struct ExprRes *doArith(struct ExprRes *Res1, struct ExprRes *Res2, char op);
struct ExprRes *doPow(struct ExprRes *base, struct ExprRes *pow);
struct ExprRes *doNegate(struct ExprRes *Expr);
void Finish(struct InstrSeq *Code);

/* Control Semantics Actions */
struct InstrSeq *doWhile(struct ExprRes *Expr, struct InstrSeq *code);
struct InstrSeq *
doIfElse(struct ExprRes *Expr, struct InstrSeq *iCode, struct InstrSeq *eCode);
struct InstrSeq *doIf(struct ExprRes *Expr, struct InstrSeq *code);

/* Functions Semantics Actions */
struct InstrSeq *doReturn(struct ExprRes *Expr);
struct ExprRes *doCall(char *name);
void doFuncInit(char *name, struct VarType *type);
struct InstrSeq *doDecFunc(char *name, struct InstrSeq *code);
struct InstrSeq *doFuncInstrs(struct ExprRes *res);
void doDecArg(struct ExprRes *res);

/* Bool Semantics Actions */
struct ExprRes *doBoolLit(int b);
struct ExprRes *doNot(struct ExprRes *Expr);
struct ExprRes *doBoolOp(struct ExprRes *Res1, struct ExprRes *Res2, int op);
struct InstrSeq *doPrintBool(struct ExprRes *Expr);

/* Int Semantics Actions */
struct ExprRes *doCompInt(struct ExprRes *Res1, struct ExprRes *Res2, int op);
struct ExprRes *doArithInt(struct ExprRes *Res1, struct ExprRes *Res2, char op);
struct ExprRes *doPowInt(struct ExprRes *base, struct ExprRes *pow);
struct ExprRes *doIntLit(int val);
struct ExprRes *doNegateInt(struct ExprRes *Expr);
struct InstrSeq *doPrintInt(struct ExprRes *Expr);
struct ExprRes *doReadInt(struct IdAddr *addr);

/* Float Semantics Actions */
struct ExprRes *doIntToFloat(struct ExprRes *Expr);
struct ExprRes *doFloatToInt(struct ExprRes *Expr);
struct ExprRes *doFloatLit(float val);
struct ExprRes *
doArithFloat(struct ExprRes *Res1, struct ExprRes *Res2, char op);
struct ExprRes *doNegateFloat(struct ExprRes *Expr);
struct InstrSeq *doPrintFloat(struct ExprRes *Expr);
struct ExprRes *doCompFloat(struct ExprRes *Res1, struct ExprRes *Res2, int op);
struct ExprRes *doReadFloat(struct IdAddr *addr);
struct ExprRes *doPowFloat(struct ExprRes *base, struct ExprRes *pow);

/* Str Semantics Actions */
struct InstrSeq *doPrintStr(struct ExprRes *Expr);
struct ExprRes *doStrLit(char *str);

/* Pointer Semantics Actions */
struct IdAddr *doIdAddr(char *name, int SZOff);
struct IdAddr *doDeRef(struct IdAddr *addr, struct ExprRes *offset);
struct ExprRes *doAddr(struct IdAddr *addr);
