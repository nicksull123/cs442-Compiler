SRCDIR=src
DIRS=SymTab IOMngr ArithInterp Comp Tests
CLN_DIRS=SymTab IOMngr ArithInterp Comp Tests

all: $(DIRS)

$(DIRS):
	$(MAKE) -C $(SRCDIR)/$@

.PHONY: clean
clean:
	rm -rf bin
