CC=clang
CFLAGS=-O3

ODIR=../../bin/SymTab

LIBS=-lm
DEPS=SymTab.h

_OBJ=SymTab.o driver.o
OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

driver: $(OBJ)
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) $(LIBS)

all: driver

.PHONY: clean

clean:
	rm -f $(ODIR)/*
