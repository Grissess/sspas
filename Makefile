CC = gcc
CCFLAGS = -g

sspas: main.o lex.yy.o parser.o tokenizer.h parser.h
	$(CC) $(CCFLAGS) -o $@ $^

main.o: main.c tokenizer.h parser.h
	$(CC) $(CCFLAGS) -c -o $@ main.c

parser.c parser.h: lemon parser.y
	./lemon parser.y

lemon: lemon.c
	$(CC) $(CCFLAGS) -o $@ $^

lex.yy.c tokenizer.h: tokenizer.l
	flex --header-file=tokenizer.h $^

makeheaders: makeheaders.c
	$(CC) $(CCFLAGS) -o $@ $^

clean:
	rm *.o lex.yy.c tokenizer.h parser.c parser.h parser.out lemon
