SRCDIR=src
DIRS=SymTab IOMngr ArithInterp Comp

all: $(DIRS)

$(DIRS):
	$(MAKE) -C $(SRCDIR)/$@

.PHONY: clean
clean:
	rm -rf bin
