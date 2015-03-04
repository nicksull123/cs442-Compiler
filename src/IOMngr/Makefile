CC=clang
CFLAGS=-O3

ODIR=../../bin/IOMngr

LIBS=
DEPS=IOMngr.h
_OBJ=IOMngr.o

OBJ=$(patsubst %, $(ODIR)/%, $(_OBJ))

all: $(OBJ)

$(ODIR)/%.o: %.c $(DEPS)
	mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*
