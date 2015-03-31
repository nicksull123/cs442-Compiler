%{
#include "../SymTab/SymTab.h"
#include "../IOMngr/IOMngr.h"
#include "y.tab.h"

#define YY_INPUT( buf, result, max_size )                      \
    {                                                          \
        int c = GetSourceChar();                               \
        result = ( c == EOF ) ? YY_NULL : ( buf[ 0 ] = c, 1 ); \
    }

%}


letter [A-Za-z]
digit [0-9]

%%
true                            {return True;}
false                           {return False;}
if                              {return IF;}
int                             {return Int;}
bool                            {return Bool;}
while                           {return While;};
print                           {return Write;}
println                         {return Writeln;}
printsp                         {return Writesp;}
read                            {return Read;};
{letter}({letter}|{digit})*     {return Ident;}
{digit}{digit}*                 {return IntLit;}
\"(\\.|[^"\\])*\"               {return Str;}
\=\=                            {return EQ;}
\<                              {return LT;}
\<\=                            {return LTE;}
\>                              {return GT;}
\>\=                            {return GTE;}
\!\=                            {return NE;}
\!                              {return NOT;}
\|\|                            {return OR;};
\&\&                            {return AND;};
\=                              {return '=';}
\+                              {return '+';}
\-                              {return '-';}
\*                              {return '*';}
\/                              {return '/';}
\%                              {return '%';}
\^                              {return '^';}
\;                              {return ';';}
\{                              {return '{';}
\}                              {return '}';}
\(                              {return '(';}
\)                              {return ')';}
\,                              {return ',';}

[ ]                             {}
\t                              {}
\r                              {}
\n                              {}

.                               {
                                    WriteIndicator(GetCurrentColumn());
                                    WriteMessage("Illegal Character in lex");
                                }

%%

int
yywrap () {
    return 1;
}
            
    
