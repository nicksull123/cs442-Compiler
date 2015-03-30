%{
#include "../IOMngr/IOMngr.h"
#include "y.tab.h"

#define YY_INPUT(buf, result, max_size)                 \
{                                                       \
    int c = GetSourceChar();                            \
    result = (c == EOF) ? YY_NULL : (buf[0] = c, 1);    \
}
%}

letter [A-Za-z]  
digit [0-9]

%%

{letter}({letter}|{digit})*         {return ident;}
[0-9]+                              {return INT;}
\=                                  {return '=';}
\;                                  {return ';';}
\(                                  {return '(';}
\)                                  {return ')';}
\+                                  {return '+';}
\*                                  {return '*';}
\-                                  {return '-';}
[ ]                                 {}
\t                                  {}
\r                                  {}
\n                                  {}

.                                   {
                                        WriteIndicator(GetCurrentColumn());
                                        WriteMessage("Illegal Character in Lex");
                                    }

%%

int 
yywrap()
{
    return 1;
}
