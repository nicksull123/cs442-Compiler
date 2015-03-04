%{
#include "semantics.h"
#include "../IOMngr/IOMngr.h"
#include <string.h>

extern int yylex();
extern char *yytext;
extern int yyleng;
extern int yyerror(char *);
%}

%union {
    int32_t integer;
    char *string;
}

%type <string> Id
%type <integer> Expr
%type <integer> Term
%type <integer> Factor

%token ident
%token INT

%%

Prog        :       StmtSeq                 {printSymTab();};
StmtSeq     :       Stmt StmtSeq            {};
StmtSeq     :                               {};
Stmt        :       Id '=' Expr ';'         {storeVar($1, $3);};
Expr        :       Expr '+' Term           {$$ = doPlus($1, $3);};
Expr        :       Term                    {$$ = $1;};
Term        :       Term '*' Factor         {$$ = doMult($1, $3);};
Term        :       Factor                  {$$ = $1;};
Factor      :       '-' Factor              {$$ = 0 - $2;};
Factor      :       '(' Expr ')'            {$$ = $2;};
Factor      :       Id                      {$$ = getVal($1);};
Factor      :       INT                     {$$ = atoi(yytext);};
Id          :       ident                   {$$ = strdup(yytext);};

%%

int
yyerror(char *s)
{
    WriteIndicator(GetCurrentColumn());
    WriteMessage("Illegal Character in YACC");
    return 1;
}
