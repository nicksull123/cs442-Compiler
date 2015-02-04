SRCDIR=src
DIRS=SymTab

all: $(DIRS)

$(DIRS):
	$(MAKE) -C $(SRCDIR)/$@

clean:
	rm -rf bin
