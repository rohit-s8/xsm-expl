CC = gcc
CFLAG = -c -w -g
LEX = flex
YACC = yacc -d
VER = 5

all: expl$(VER) linker


expl$(VER): expl.o node.o lexer.o parser.o files.o
	$(CC) -g -o expl$(VER) expl.o node.o lexer.o parser.o files.o

linker: labeltranslator.o pair.o files.o
	$(CC) -g -o linker labeltranslator.o pair.o files.o

expl.o: expl.c node.h types.h y.tab.h
	$(CC) $(CFLAG) expl.c

node.o: node.c node.h
	$(CC) $(CFLAG) node.c

lexer.o: lex.yy.c y.tab.h node.h
	$(CC) $(CFLAG) -o lexer.o lex.yy.c

parser.o: y.tab.c y.tab.h node.h
	$(CC) $(CFLAG) -o parser.o y.tab.c

pair.o: pair.c pair.h
	$(CC) $(CFLAG) pair.c

labeltranslator.o: labeltranslator.c types.h pair.h
	$(CC) $(CFLAG) labeltranslator.c

files.o: files.c
	$(CC) $(CFLAG) files.c

y.tab.c y.tab.h: explparser.y
	$(YACC) explparser.y

lex.yy.c: expllexer.l
	$(LEX) expllexer.l

labeltranslator.c: labellexer.l
	$(LEX) -o labeltranslator.c labellexer.l

clean:
	rm *.o lex.yy.c y.tab.c y.tab.h labeltranslator.c expl$(VER) linker

