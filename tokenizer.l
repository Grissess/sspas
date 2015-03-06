%{
#include "parser.h"
#include <stdio.h>
%}

letter	[a-zA-Z]
digit	[0-9]

%%

{digit}+(\.{digit}+)?(E(\+|\-)?{digit}+)?	{ return NUM; }

program { return PROGRAM; }
var { return VAR; }
array { return ARRAY; }
of { return OF; }
integer { return INTEGER; }
real { return REAL; }
function { return FUNCTION; }
procedure { return PROCEDURE; }
begin { return BEGIN_; }
end { return END; }
if { return IF; }
then { return THEN; }
else { return ELSE; }
while { return WHILE; }
do { return DO; }
not { return NOT; }

{letter}({letter}|{digit})* { return IDENT; }

(=|<|>|<=|>=|<>) { return RELOP; }
(\+|\-) { return ADDOP; }
(\*|\/|div|mod|and) { return MULOP; }
:= { return ASSIGNOP; }

\( { return LPAREN; }
\) { return RPAREN; }
\[ { return LBRACKET; }
\] { return RBRACKET; }
\: { return COLON; }
\; { return SEMICOLON; }
\, { return COMMA; }
\. { return DOT; }
\.\. { return DOTDOT; }

[ \t\n\v\b]+ ;

<<EOF>> { return 0; }

%%

void yyerror(const char *msg) {
	fputs(msg, stderr);
}

int yywrap(void) {
	return 1;
}
