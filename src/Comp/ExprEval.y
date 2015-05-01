%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"
#include "semantics.h"
#include "codegen.h"

int yylex(); /* The next token function. */
int yyerror(const char *);

extern struct SymTab *table;
extern struct SymEntry *entry;

%}

%glr-parser

%union {
  int val;
  float fval;
  char * string;
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct BExprRes * BExprRes;
  struct VarType *vType;
  struct IdAddr *IdAddr;
}

%type <IdAddr> IdAddr
%type <vType> Ty
%type <vType> Type
%type <string> Id
%type <string> Str
%type <val> IntLit
%type <fval> FloatLit
%type <ExprRes> Factor
%type <ExprRes> Term
%type <ExprRes> ETerm
%type <ExprRes> NTerm
%type <ExprRes> Expr
%type <InstrSeq> StmtSeq
%type <InstrSeq> Stmt
%type <ExprRes> BExpr
%type <ExprRes> BHExpr
%type <ExprRes> OExpr
%type <ExprRes> AExpr
%type <ExprRes> FuncCall
%type <InstrSeq> PVarSeq
%type <InstrSeq> RVarSeq
%type <InstrSeq> RVar
%type <InstrSeq> FuncSeq
%type <InstrSeq> FuncDec

%token Sbrk
%token Return
%token Id 
%token IntLit   
%token FloatLit
%token Int
%token Bool
%token Float
%token Str
%token Void
%token NOT
%token OR
%token AND
%token True
%token False
%token Write
%token Writeln
%token Writesp
%token Read
%token While
%token IF
%token ELSE
%token EQ   
%token LT
%token LTE
%token GT
%token GTE
%token NE

%%

Prog            :   DecsCompl FuncSeq                                       {Finish($2);};
DecsCompl       :   Declarations                                            {doPushDecs();};
Declarations    :   Dec Declarations                                        { };
                |                                                           { };
Dec             :   Type Id ';'                                             {doDeclare($2, $1, 0, 1); };
                |   Ty '[' IntLit ']' Id ';'                                {doDeclare($5, $1, 0, $3); };
FuncSeq         :   FuncDec FuncSeq                                         {$$ = AppendSeq($1, $2);};
                |                                                           {$$ = NULL;};
FuncDec         :   Type Id '(' Params ')' '{' DecsCompl                    {doFuncInit($2, $1);}
                    StmtSeq '}'                                             {$$ = doDecFunc($2, $9);};
Params          :   Param ',' Params                                        { };
                |   Param                                                   { };
                |                                                           { };
Param           :   Type Id                                                 {doDeclare($2, $1, 1, 1);};
StmtSeq         :   Stmt StmtSeq                                            {$$ = AppendSeq($1, $2); };
                |                                                           {$$ = NULL; };
Stmt            :   Return AExpr ';'                                        {$$ = doReturn($2);};
                |   FuncCall ';'                                            {$$ = doFuncInstrs($1);};
                |   While '(' AExpr ')' '{' StmtSeq '}'                     {$$ = doWhile($3, $6); };
                |   IF '(' AExpr ')' '{' StmtSeq '}'                        {$$ = doIf($3, $6);};
                |   IF '(' AExpr ')' '{' StmtSeq '}' ELSE '{' StmtSeq '}'   {$$ = doIfElse($3, $6, $10);};
                |   Read '(' RVarSeq ')' ';'                                {$$ = $3;};
                |   Writesp '(' AExpr ')' ';'                               {$$ = doPrintSp($3);};
                |   Writeln ';'                                             {$$ = doPrintLn();};
                |   Write '(' PVarSeq ')' ';'                               {$$ = $3;};
                |   IdAddr '=' AExpr ';'                                    {$$ = doAssign($1, $3, 0);};
RVarSeq         :   RVar ',' RVarSeq                                        {$$ = AppendSeq($1, $3);};
                |   RVar                                                    {$$ = $1;};
RVar            :   IdAddr                                                  {$$ = doRead($1);};
PVarSeq         :   AExpr ',' PVarSeq                                       {$$ = doPrintList($1, $3);};
                |   AExpr                                                   {$$ = doPrint($1);};
AExpr           :   AExpr OR OExpr                                          {$$ = doBoolOp($1, $3, B_OR); };
                |   OExpr                                                   {$$ = $1;};
OExpr           :   OExpr AND BHExpr                                        {$$ = doBoolOp($1, $3, B_AND); };
                |   BHExpr                                                  {$$ = $1;};
BHExpr          :   BHExpr EQ BExpr                                         {$$ = doComp($1, $3, B_EQ);};
                |   BHExpr NE BExpr                                         {$$ = doComp($1, $3, B_NE);};
                |   BExpr                                                   {$$ = $1;};
BExpr           :   BExpr LT Expr                                           {$$ = doComp($1, $3, B_LT);};
                |   BExpr LTE Expr                                          {$$ = doComp($1, $3, B_LTE);};
                |   BExpr GT Expr                                           {$$ = doComp($1, $3, B_GT);};
                |   BExpr GTE Expr                                          {$$ = doComp($1, $3, B_GTE);};
                |   Expr                                                    {$$ = $1;};
Expr            :   Expr '+' Term                                           {$$ = doArith($1, $3, '+'); };
                |   Expr '-' Term                                           {$$ = doArith($1, $3, '-'); };
                |   Term                                                    {$$ = $1; };
Term            :   Term '*' ETerm                                          {$$ = doArith($1, $3, '*'); };
                |   Term '/' ETerm                                          {$$ = doArith($1, $3, '/'); };
                |   Term '%' ETerm                                          {$$ = doArith($1, $3, '%'); };
                |   ETerm                                                   {$$ = $1; }
ETerm           :   NTerm '^' ETerm                                         {$$ = doPow($1, $3); };
                |   NTerm                                                   {$$ = $1; };
NTerm           :   '-' Factor                                              {$$ = doNegate($2); };
                |   Factor                                                  {$$ = $1; };
Factor          :   IntLit                                                  {$$ = doIntLit($1); };
                |   FloatLit                                                {$$ = doFloatLit($1);};
                |   '&' IdAddr                                              {$$ = doAddr($2);};
                |   IdAddr                                                  {$$ = doRval($1); };
                |   '(' AExpr ')'                                           {$$ = $2; };
                |   NOT Factor                                               {$$ = doNot($2);};
                |   True                                                    {$$ = doBoolLit(B_TRUE);};
                |   False                                                   {$$ = doBoolLit(B_FALSE);};
                |   Str                                                     {$$ = doStrLit($1);};
                |   FuncCall                                                {$$ = $1;};
                |   Sbrk '(' AExpr ')'                                      {$$ = doSbrk($3);};
                |   Float '(' AExpr ')'                                     {$$ = doIntToFloat($3);};
                |   Int '(' AExpr ')'                                       {$$ = doFloatToInt($3);};
FuncCall        :   Id '(' Args  ')'                                        {$$ = doCall($1);};
Args            :   Arg ',' Args                                            { };            
                |   Arg                                                     { };
                |                                                           { };
Arg             :   AExpr                                                   {doDecArg($1); };
IdAddr          :   Id                                                      {$$ = doIdAddr($1, 0);};
                |   Id '[' AExpr ']'                                        {$$ = doDeRef(doIdAddr($1, 0), $3);};
                |   '*' Id                                                  {$$ = doDeRef(doIdAddr($2, 0), NULL);};
Type            :   Ty '*'                                                  {  
                                                                                $$ = $1;
                                                                                $1->isRef = 1;
                                                                            };
                |   Ty '[' ']'                                              {
                                                                                $$ = $1;
                                                                                $1->isRef = 1;
                                                                            };
                |   Ty                                                      {$$ = $1;};
Ty              :   Bool                                                    {$$ = doVarType(T_BOOL);};
                |   Int                                                     {$$ = doVarType(T_INT);};
                |   Float                                                   {$$ = doVarType(T_FLOAT);};
                |   Void                                                    {$$ = doVarType(T_VOID);};
 
%%

int yyerror(const char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
