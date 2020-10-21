%{
    #include "node.h"
    #include "parser.h"
    NProgram *programBlock; /* the top level root node of our final AST */

    extern int yylex();
    void yyerror(const char *s) { printf("ERROR: %s\nat: %s", s, currentFile); }
%}

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NVariableDeclaration *var_decl;
    NConstDeclaration *const_decl;
    NInternalDeclaration *internal_decl;
    NSuperDeclaration *super_decl;
    std::vector<NVariableDeclaration*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV TDQUOTE TSQUOTE

%token  <token>         TCOMMENT TROCKET TPACKAGE TREQUIRE TVAR TRETURN TCHAR
%token  <token>         TCONST TINTERNAL TTHING TSUPER TFUNC TRSQUARE TLSQUARE
%token  <token>         TSTRTYPE TINTTYPE TFLOATTYPE

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric expr string string_chars
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl const_decl internal_decl super_decl thing_decl require_decl ret
%type <token> comparison

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

/*
 Track locations of symbols to allow for better error messages
 in the compiler
*/
%locations
%define parse.error custom

%start program

%%

program : TPACKAGE ident stmts { programBlock = new NProgram(currentFile, @$.first_line, @$.first_column, *$2, *$3); }
        ;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
      | stmts stmt { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl | const_decl | internal_decl | super_decl | thing_decl
     | expr { $$ = new NExpressionStatement(*$1); }
     | TRETURN expr { $$ = new NReturn(*$2); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
      | TLBRACE TRBRACE { $$ = new NBlock(); }
      ;

require_decl : TREQUIRE TCLT ident TCGT { $$ = new NStdlibRequirementDeclaration(*$1); }
             | TREQUIRE TDQUOTE ident TDQUOTE { $$ = new NRelativeRequirementDeclaration(*$1); }
             ;

var_decl : TVAR ident ident TEQUAL expr { $$ = new NVariableDeclaration(*$2, *$3, $5); }
         | TVAR ident ident { $$ = new NVariableDeclaration(*$2, *$3); }
         | ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
         ;

const_decl : TCONST ident ident TEQUAL expr { $$ = new NConstDeclaration(*$2, *$3, $5); }
           | TCONST ident ident { $$ = new NConstDeclaration(*$2, *$3); }
           ;

internal_decl : TINTERNAL ident ident TEQUAL expr { $$ = new NInternalDeclaration(*$2, *$3, $5); }
              ;

super_decl : TSUPER ident { $$ = new NSuperDeclaration(*$2); }
           ;

thing_decl : TTHING ident block { $$ = new NThingDeclaration(*$2, *$3); }
         ;

func_decl : TFUNC ident TLPAREN func_decl_args TRPAREN TROCKET TLPAREN ident TRPAREN block
 { $$ = new NFunctionDeclaration(*$2, *$4, *$8, *$10); delete $4; }
          ;

func_decl_args : /*blank*/  { $$ = new VariableList(); }
          | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

ident : TIDENTIFIER { $$ = new NIdentifier(currentFile, @$.first_line, @$.first_column, *$1); delete $1; }
      ;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
        | TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
        ;

string : TDQUOTE string_chars TDQUOTE { $$ = new NString(*$2); }
       ;

string_chars : TCHAR | string_chars TCHAR
             ;

expr : ident TEQUAL expr { $$ = new NAssignment(currentFile, @$.first_line, @$.first_column, *$<ident>1, *$3); }
     | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
     | ident { $<ident>$ = $1; }
     | numeric
     | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
    ;

call_args : /*blank*/  { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
           | TPLUS | TMINUS | TMUL | TDIV
           ;

%%

/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)                  \
     fprintf (File, "line: %d, col: %d",                      \
              (Loc).first_line, (Loc).first_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif

// Custom syntax error report. See: https://www.gnu.org/software/bison/manual/html_node/Syntax-Error-Reporting-Function.html
// (which is where this function is lifted almost verbatim from)
static int
yyreport_syntax_error (const yypcontext_t *ctx)
{
  int res = 0;
  fprintf(stderr, "%s, ", currentFile);
  YY_LOCATION_PRINT (stderr, *yypcontext_location (ctx));
  fprintf (stderr, ": syntax error");
  // Report the tokens expected at this point.
  {
    enum { TOKENMAX = 5 };
    yysymbol_kind_t expected[TOKENMAX];
    int n = yypcontext_expected_tokens (ctx, expected, TOKENMAX);
    if (n < 0)
      // Forward errors to yyparse.
      res = n;
    else
      for (int i = 0; i < n; ++i)
        fprintf (stderr, "%s %s",
                 i == 0 ? ": expected" : " or", yysymbol_name (expected[i]));
  }
  // Report the unexpected token.
  {
    yysymbol_kind_t lookahead = yypcontext_token (ctx);
    if (lookahead != YYSYMBOL_YYEMPTY)
      fprintf (stderr, " before %s", yysymbol_name (lookahead));
  }
  fprintf (stderr, "\n");
  return res;
}
