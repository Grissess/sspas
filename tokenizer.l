%{
#include "parser.h"
#include <stdio.h>

#define NEW(ty) (malloc(sizeof(ty)))
#define AS(ty, ex) ((ty *) (ex))

void *semval;
%}

letter	[a-zA-Z]
digit	[0-9]

%%

\(\*.*\*\) ;

(\+|\-)?{digit}+ { semval = NEW(long); *AS(long, semval) = atol(yytext); return TOK_LIT_INTEGER; }
(\+|\-)?{digit}+\.{digit}+([Ee](\+|\-)?{digit}+)?	{ semval = NEW(double); *AS(double, semval) = atof(yytext); return TOK_LIT_REAL; }
\'[^']\' { semval = NEW(char); *AS(char, semval) = yytext[1]; return TOK_LIT_CHAR; }

program { return TOK_PROGRAM; }
var { return TOK_VAR; }
array { return TOK_ARRAY; }
of { return TOK_OF; }
integer { return TOK_INTEGER; }
real { return TOK_REAL; }
character { return TOK_CHARACTER; }
function { return TOK_FUNCTION; }
procedure { return TOK_PROCEDURE; }
begin { return TOK_BEGIN; }
end { return TOK_END; }
if { return TOK_IF; }
then { return TOK_THEN; }
else { return TOK_ELSE; }
while { return TOK_WHILE; }
do { return TOK_DO; }
for { return TOK_FOR; }
in { return TOK_IN; }
not { return TOK_NOT; }
div { return TOK_DIV; }
mod { return TOK_MOD; }
and { return TOK_AND; }
or { return TOK_OR; }
to { return TOK_TO; }
type { return TOK_TYPE; }

({letter}|_)({letter}|{digit}|_)* { semval = strdup(yytext); return TOK_IDENT; }

\= { return TOK_EQ; }
\< { return TOK_LESS; }
\> { return TOK_GREATER; }
\<\= { return TOK_LEQ; }
\>\= { return TOK_GEQ; }
\<\> { return TOK_NEQ; }
\+ { return TOK_ADD; }
\- { return TOK_SUB; }
\* { return TOK_MUL; }
\/ { return TOK_DIV; }
\% { return TOK_MOD; }
\& { return TOK_BAND; }
\| { return TOK_BOR; }
\^ { return TOK_BXOR; }
\<\< { return TOK_BLSHIFT; }
\>\> { return TOK_BRSHIFT; }
\~ { return TOK_BNOT; }
\:\= { return TOK_ASSIGN; }
\-\> { return TOK_ARROW; }
\( { return TOK_LPAREN; }
\) { return TOK_RPAREN; }
\[ { return TOK_LBRACKET; }
\] { return TOK_RBRACKET; }
\{ { return TOK_LBRACE; }
\} { return TOK_RBRACE; }
\: { return TOK_COLON; }
\; { return TOK_SEMICOLON; }
\, { return TOK_COMMA; }
\. { return TOK_DOT; }
\.\. { return TOK_DOTDOT; }

[ \t\n\v\b]+ ;

<<EOF>> { return 0; }

%%

void yyerror(const char *msg) {
	fputs(msg, stderr);
}

int yywrap(void) {
	return 1;
}
