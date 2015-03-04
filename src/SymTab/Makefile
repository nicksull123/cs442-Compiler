CC=clang
CFLAGS=-O3

ODIR=../../bin/SymTab

LIBS=
DEPS=SymTab.h
_OBJ=SymTab.o driver.o

OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

all: driver

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

driver: $(OBJ)
	$(CC) -o $(ODIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*
