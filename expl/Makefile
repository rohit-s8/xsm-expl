CC = gcc
CFLAG = -c -w
LEX = flex
YACC = yacc -d
VER =

all: expl$(VER) linker


expl$(VER): expl.o node.o lexer.o parser.o files.o types.o class.o
	$(CC) -o expl$(VER) expl.o node.o lexer.o parser.o files.o types.o\
		class.o

linker: labeltranslator.o pair.o files.o
	$(CC) -o linker labeltranslator.o pair.o files.o

expl.o: expl.c node.h types.h class.h y.tab.h
	$(CC) $(CFLAG) expl.c

node.o: node.c node.h structs.h
	$(CC) $(CFLAG) node.c

class.o: class.c class.h structs.h
	$(CC) $(CFLAG) class.c

types.o: types.c types.h structs.h
	$(CC) $(CFLAG) -o types.o types.c

lexer.o: lex.yy.c y.tab.h
	$(CC) $(CFLAG) -o lexer.o lex.yy.c

parser.o: y.tab.c y.tab.h
	$(CC) $(CFLAG) -o parser.o y.tab.c

pair.o: pair.c pair.h
	$(CC) $(CFLAG) pair.c

labeltranslator.o: labeltranslator.c pair.h
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

