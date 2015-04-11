CC=clang
CFLAGS=-O3

OPATHS=$(shell find ../../bin -type d)
vpath %.o $(OPATHS)

ODIR=../../bin/Comp

LIBS=
OLINK=SymTab.o IOMngr.o
DEPS=semantics.h ExprEval.y lex1.l
_OBJ=y.tab.o main.o semantics.o lex.yy.o codegen.o semantics_bool.o semantics_int.o semantics_str.o semantics_ctrl.o semantics_funcs.o semantics_ptr.o semantics_float.o libclite.o

OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

all: Comp

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

Comp: $(OBJ) $(OLINK)
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) $(LIBS)

y.tab.c: ExprEval.y
	yacc -d $<

lex.yy.c: lex1.l
	lex $<

libclite.c: libclite.cl
	xxd -i $< > $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*
	rm -f y.tab.c y.tab.h lex.yy.c libclite.c

