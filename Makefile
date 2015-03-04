SRCDIR=src
DIRS=SymTab IOMngr ArithInterp

all: $(DIRS)

$(DIRS):
	$(MAKE) -C $(SRCDIR)/$@

.PHONY: clean
clean:
	rm -rf bin
