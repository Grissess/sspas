#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "parser.h"
#include "ast.h"

#include "toknames.c"

extern void *semval;

void *ParseAlloc(void *(*)(size_t));
void ParseFree(void *, void (*)(void *));
void Parse(void *, int, void *, ast_root *);
void ParseTrace(FILE *, char *);

int main(int argc, char **argv) {
		FILE *input = stdin;
		YY_BUFFER_STATE yybuf;
		void *parser;
		int token;
		ast_root ast;
		ast.prog = NULL;

		if(argc > 2) {
				fprintf(stderr, "Usage: %s [<infile>]\n\nInput defaults to standard input.\n", argv[0]);
				return 1;
		}
		if(argc > 1) {
				input = fopen(argv[1], "r");
				if(!input) {
						fprintf(stderr, "Failed to open input file.\n");
						return 1;
				}
		}

		yybuf = yy_create_buffer(input, YY_BUF_SIZE);
		yy_switch_to_buffer(yybuf);

		parser = ParseAlloc(malloc);
		ParseTrace(stderr, "parser: ");
		while(token = yylex()) {
				fprintf(stderr, " [%s] ", toknames[token]);
				Parse(parser, token, semval, &ast);
		}
		Parse(parser, 0, NULL, &ast);
		ParseFree(parser, free);

		if(ast.prog)
			prog_print(stderr, 0, ast.prog);
		else
			fprintf(stderr, "NULL object.\n");

		return 0;
}
