%{
#include <string>
#include "node.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }
%}

%%

"//"                    return TOKEN(TCOMMENT);
"->"                    return TOKEN(TROCKET);
"package"               return TOKEN(TPACKAGE);
"require"               return TOKEN(TREQUIRE);
"var"                   return TOKEN(TVAR);
"const"                 return TOKEN(TCONST);
"internal"              return TOKEN(TINTERNAL);
"thing"                 return TOKEN(TTHING);
"super"                 return TOKEN(TSUPER);
"f"                     return TOKEN(TFUNC);
"return"                return TOKEN(TRETURN);

[ \t\n]                 ;
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TIDENTIFIER;
[0-9]+.[0-9]*           SAVE_TOKEN; return TDOUBLE;
[0-9]+                  SAVE_TOKEN; return TINTEGER;

"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
"["                     return TOKEN(TLSQUARE);
"]"                     return TOKEN(TRSQUARE);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
"\""                    return TOKEN(TDQUOTE);
"'"                     return TOKEN(TSQUOTE);


.                       printf("Unknown token!n"); yyterminate();

%%
