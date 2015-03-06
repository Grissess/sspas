%include {#include <stdio.h>}
%include {#include <assert.h>}



program ::= PROGRAM IDENT LPAREN identifier_list RPAREN SEMICOLON declarations subprogram_declarations compound_statement DOT.
	
identifier_list ::= IDENT.
identifier_list ::= identifier_list COMMA IDENT.

declarations ::= declarations VAR identifier_list COLON type SEMICOLON.
declarations ::= .

type ::= standard_type.
type ::= ARRAY LBRACKET NUM DOTDOT NUM RBRACKET OF standard_type.

standard_type ::= INTEGER.
standard_type ::= REAL.

subprogram_declarations ::= subprogram_declarations subprogram_declaration.
subprogram_declarations ::= .

subprogram_declaration ::= subprogram_head declarations compound_statement.

subprogram_head ::= FUNCTION IDENT arguments COLON standard_type SEMICOLON.
subprogram_head ::= PROCEDURE IDENT arguments SEMICOLON.

arguments ::= LPAREN parameter_list RPAREN.
arguments ::= .

parameter_list ::= identifier_list COLON type.
parameter_list ::= parameter_list SEMICOLON identifier_list COLON type.

compound_statement ::= BEGIN_ optional_statements END.

optional_statements ::= statement_list.
optional_statements ::= .

statement_list ::= statement.
statement_list ::= statement_list SEMICOLON statement.

statement ::= variable ASSIGNOP expression.
statement ::= procedure_statement.
statement ::= compound_statement.
statement ::= IF expression THEN statement ELSE statement.
statement ::= WHILE expression DO statement.

variable ::= IDENT.
variable ::= IDENT LBRACKET expression RBRACKET.

procedure_statement ::= IDENT.
procedure_statement ::= IDENT LPAREN expression_list RPAREN.

expression_list ::= expression.
expression_list ::= expression_list COMMA expression.

expression ::= simple_expression.
expression ::= simple_expression RELOP simple_expression.

simple_expression ::= term.
simple_expression ::= sign term.
simple_expression ::= simple_expression ADDOP term.

term ::= factor.
term ::= term MULOP factor.

factor ::= IDENT.
factor ::= IDENT LPAREN expression_list RPAREN.
factor ::= NUM.
factor ::= LPAREN expression RPAREN.
factor ::= NOT factor.

sign ::= ADDOP.



%parse_accept {
	fprintf(stderr, "Syntax check OK\n");
}

%parse_failure {
	fprintf(stderr, "Syntax check BAD\n");
}
