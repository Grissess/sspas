CC = gcc
CCFLAGS = -g

sspas: ast.o vector.o util.o lit.o main.o type.o lex.yy.o parser.o tokenizer.h parser.h
	$(CC) $(CCFLAGS) -o $@ $^

main.o: main.c toknames.c tokenizer.h parser.h
	$(CC) $(CCFLAGS) -c -o $@ main.c

ast.o: ast.c ast.h
	$(CC) $(CCFLAGS) -c -o $@ ast.c

lit.o: lit.c lit.h
	$(CC) $(CCFLAGS) -c -o $@ lit.c

type.o: type.c type.h
	$(CC) $(CCFLAGS) -c -o $@ type.c

vector.o: vector.c vector.h
	$(CC) $(CCFLAGS) -c -o $@ vector.c

util.o: util.c util.h
	$(CC) $(CCFLAGS) -c -o $@ util.c

toknames.c: parser.h
	python mktoknames.py

parser.c parser.h: lemon parser.y
	./lemon -s parser.y || true

lemon: lemon.c
	$(CC) $(CCFLAGS) -o $@ $^

lex.yy.c tokenizer.h: tokenizer.l
	flex --header-file=tokenizer.h $^

makeheaders: makeheaders.c
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm *.o lex.yy.c tokenizer.h parser.c parser.h parser.out lemon
