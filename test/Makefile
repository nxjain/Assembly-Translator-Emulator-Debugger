CC     ?= gcc
CFLAGS ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -pedantic

TESTBINDIR=bin
TESTOBJDIR=obj

SRCDIR=../src
SRCOBJDIR=../obj

EMUDIR=emulator
ASMDIR=assembler
EXTDIR=extension
ADTDIR=ADTs

#finds all c files recursively through all files except for ./Unity directory
TESTSRCS=$(shell find . -path ./Unity -prune -o -name "*.c" -print)
TESTS=$(patsubst %.c, $(TESTBINDIR)/%, $(notdir $(TESTSRCS)))


.PHONY: all clean

all: $(TESTOBJDIR) $(TESTBINDIR) $(TESTS)

$(TESTOBJDIR):
	mkdir -p $@
$(TESTBINDIR):
	mkdir -p $@

#Link the object files
$(TESTBINDIR)/testhashmap: $(SRCOBJDIR)/hashmap.o $(TESTOBJDIR)/testhashmap.o $(TESTOBJDIR)/unity.o
	$(CC) $(CFLAGS) $^ -o $@
$(TESTBINDIR)/test%: $(TESTOBJDIR)/test%.o $(SRCOBJDIR)/%.o $(TESTOBJDIR)/unity.o
	$(CC) $(CFLAGS) $^ -o $@

#Creating object files
$(TESTOBJDIR)/unity.o: Unity/src/unity.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TESTOBJDIR)/testymbol_table.o:: testymbol_table.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TESTOBJDIR)/test%.o:: test%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TESTOBJDIR)/test%.o:: $(EMUDIR)/test%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TESTOBJDIR)/test%.o:: $(ASMDIR)/test%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TESTOBJDIR)/test%.o:: $(EXTDIR)/test%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TESTOBJDIR)/test%.o:: $(ADTDIR)/test%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TESTBINDIR)/* $(TESTOBJDIR)/*