%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"
#include "semantics.h"
#include "codegen.h"

extern int yylex(); /* The next token function. */
extern char *yytext;   /* The matched token text.  */
extern int yyleng;      /* The token text length.   */
extern int yyparse();
extern int yyerror(char *);
void dumpTable();

extern struct SymTab *table;
extern struct SymEntry *entry;

%}


%union {
  long val;
  char * string;
  struct ExprRes * ExprRes;
  struct InstrSeq * InstrSeq;
  struct BExprRes * BExprRes;
}

%type <string> Id
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

%token Ident        
%token IntLit   
%token Int
%token Bool
%token NOT
%token OR
%token AND
%token True
%token False
%token Write
%token IF
%token EQ   
%token LT
%token LTE
%token GT
%token GTE
%token NE

%%

Prog            :   Declarations StmtSeq                    {Finish($2); } ;
Declarations    :   Dec Declarations                        { };
Declarations    :                                           { };
Dec             :   Int Ident                               {doDeclare(yytext, T_INT); } ';' {};
Dec             :   Bool Ident                              {doDeclare(yytext, T_BOOL); } ';' {};
StmtSeq         :   Stmt StmtSeq                            {$$ = AppendSeq($1, $2); };
StmtSeq         :                                           {$$ = NULL; };
Stmt            :   Write AExpr ';'                         {$$ = doPrint($2); };
Stmt            :   Id '=' AExpr ';'                        {$$ = doAssign($1, $3); };
AExpr           :   AExpr AND OExpr                         {$$ = doBoolOp($1, $3, B_AND); };
AExpr           :   OExpr                                   {$$ = $1;};
OExpr           :   OExpr OR BHExpr                         {$$ = doBoolOp($1, $3, B_OR); };
OExpr           :   BHExpr                                  {$$ = $1;};
BHExpr          :   BHExpr EQ BExpr                         {$$ = doComp($1, $3, B_EQ);};
BHExpr          :   BHExpr NE BExpr                         {$$ = doComp($1, $3, B_NE);};
BHExpr          :   BExpr                                   {$$ = $1;};
BExpr           :   BExpr LT Expr                           {$$ = doComp($1, $3, B_LT);};
BExpr           :   BExpr LTE Expr                          {$$ = doComp($1, $3, B_LTE);};
BExpr           :   BExpr GT Expr                           {$$ = doComp($1, $3, B_GT);};
BExpr           :   BExpr GTE Expr                          {$$ = doComp($1, $3, B_GTE);};
BExpr           :   Expr                                    {$$ = $1;};
Expr            :   Expr '+' Term                           {$$ = doArith($1, $3, '+'); };
Expr            :   Expr '-' Term                           {$$ = doArith($1, $3, '-'); };
Expr            :   Term                                    {$$ = $1; };
Term            :   Term '*' ETerm                          {$$ = doArith($1, $3, '*'); };
Term            :   Term '/' ETerm                          {$$ = doArith($1, $3, '/'); };
Term            :   Term '%' ETerm                          {$$ = doArith($1, $3, '%'); };
Term            :   ETerm                                   {$$ = $1; }
ETerm           :   NTerm '^' ETerm                         {$$ = doPow($1, $3); };
ETerm           :   NTerm                                   {$$ = $1; };
NTerm           :   '-' Factor                              {$$ = doNegate($2); };
NTerm           :   Factor                                  {$$ = $1; };
Factor          :   IntLit                                  {$$ = doIntLit(yytext); };
Factor          :   Ident                                   {$$ = doRval(yytext); };
Factor          :   '(' AExpr ')'                           {$$ = $2; };
Factor          :   NOT AExpr                               {$$ = doNot($2);};
Factor          :   True                                    {$$ = doBoolLit(B_TRUE);};
Factor          :   False                                   {$$ = doBoolLit(B_FALSE);};
Id              :   Ident                                   {$$ = strdup(yytext); };
 
%%

int yyerror(char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
