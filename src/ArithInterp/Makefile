CC=clang
CFLAGS=-O3

OPATHS=$(shell find ../../bin -type d)
vpath %.o $(OPATHS)

ODIR=../../bin/ArithInterp

LIBS=
OLINK=SymTab.o IOMngr.o
DEPS=semantics.h arithLex.l arithYacc.y arithLex.l
_OBJ=main.o semantics.o y.tab.o lex.yy.o

OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

all: arithInterp

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

arithInterp: $(OBJ) $(OLINK)
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) $(LIBS)

y.tab.c: arithYacc.y
	yacc -d $<

lex.yy.c: arithLex.l
	lex $<

.PHONY: clean
clean:
	rm -f $(ODIR)/*
	rm -f y.tab.c y.tab.h lex.yy.c

