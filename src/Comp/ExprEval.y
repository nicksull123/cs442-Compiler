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
%type <ExprRes> BBExpr
%type <ExprRes> VExpr

%token Ident        
%token IntLit   
%token Int
%token Bool
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
Stmt            :   Write VExpr ';'                         {$$ = doPrint($2); };
Stmt            :   Id '=' VExpr ';'                        {$$ = doAssign($1, $3); };
VExpr           :   BBExpr                                  {$$ = $1;};
VExpr           :   Expr                                    {$$ = $1;};
//Stmt          :   IF '(' BExpr ')' '{' StmtSeq '}'        {$$ = doIf($3, $6); };
BBExpr          :   BExpr                                   {$$ = $1;};
BBExpr          :   True                                    {$$ = doBoolLit(B_TRUE);};
BBExpr          :   False                                   {$$ = doBoolLit(B_FALSE);};
BBExpr          :   Id                                      {$$ = doRval(yytext);};
BExpr           :   Expr EQ Expr                            {$$ = doBExpr($1, $3, B_EQ);};
BExpr           :   Expr LT Expr                            {$$ = doBExpr($1, $3, B_LT);};
BExpr           :   Expr LTE Expr                           {$$ = doBExpr($1, $3, B_LTE);};
BExpr           :   Expr GT Expr                            {$$ = doBExpr($1, $3, B_GT);};
BExpr           :   Expr GTE Expr                           {$$ = doBExpr($1, $3, B_GTE);};
BExpr           :   Expr NE Expr                            {$$ = doBExpr($1, $3, B_NE);};
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
Factor          :   '(' Expr ')'                            {$$ = $2; };
Id              :   Ident                                   {$$ = strdup(yytext); };
 
%%

int yyerror(char *s)  {
  WriteIndicator(GetCurrentColumn());
  WriteMessage("Illegal Character in YACC");
  return 1;
}
