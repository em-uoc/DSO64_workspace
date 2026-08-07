
PROGS   = test_nch
LDFLAGS = -static

all: $(PROGS)

test_nch: test_nch.o get_nch.o
test_nch.o: get_nch.h

clean:
	$(RM) $(PROGS) *.o

.PHONY: clean
