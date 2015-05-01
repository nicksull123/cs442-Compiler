ODIR=../../bin/Tests

_TSTS=test_a.asm test_b.asm test_c.asm test_d.asm test_e.asm test_f.asm test_first_sec.asm test_g.asm test_h.asm test_i.asm test_k.asm test_l.asm test_m.asm test_n.asm test_o.asm test_p.asm test_q.asm test_r.asm test_s.asm test_t.asm test_nested.asm test_u.asm gendreau1.asm gendreau2.asm gendreau3.asm gendreau4.asm gendreau5.asm

TSTS=$(patsubst %, $(ODIR)/%, $(_TSTS))

all: tests

$(ODIR)/%.asm: %
	mkdir -p $(ODIR)
	../../bin/Comp/Comp $< $@

tests: $(TSTS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*
