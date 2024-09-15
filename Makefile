CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -pedantic

SRCDIR=src
OBJDIR=obj

EMUDIR=emulator
ASMDIR=assembler
EXTDIR=extension
ADTDIR=ADTs

SOLUTIONDIR=armv8_testsuite/solution

#finds all c files recursively in the SRCDIR
SRCS=$(shell find $(SRCDIR) -name "*.c")
OBJS=$(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRCS)))

BINDIR=bin
BINS=$(BINDIR)/assemble $(BINDIR)/emulate $(BINDIR)/debugger
TESTDIR=test
TESTBINDIR=test/bin
DOCDIR=doc
LATEXDIR=doc/doxygen/latex
DOCOUTDIR=doc/out
PDFNAME=refman
LEDBLINKDIR=led_blink


.PHONY: all clean test

all: $(BINDIR) $(OBJDIR) $(BINS) $(SOLUTIONDIR)
	cp $(BINDIR)/assemble $(BINDIR)/emulate $(SOLUTIONDIR)

#Create BINDIR and OBJDIR if it does not exist
$(BINDIR):
	mkdir -p $@
$(OBJDIR):
	mkdir -p $@
$(SOLUTIONDIR):
	mkdir -p $@

#Link the object files
$(BINDIR)/assemble: $(OBJDIR)/symbol_table.o $(OBJDIR)/decode_helper.o $(OBJDIR)/darray.o $(OBJDIR)/hashmap.o $(OBJDIR)/utils.o $(OBJDIR)/decode.o $(OBJDIR)/assemble.o 
	$(CC) $(CFLAGS) $^ -o $@
$(BINDIR)/emulate: $(OBJDIR)/darray.o $(OBJDIR)/hashmap.o $(OBJDIR)/utils.o $(OBJDIR)/memory.o $(OBJDIR)/register.o $(OBJDIR)/cpu.o $(OBJDIR)/emulate.o 
	$(CC) $(CFLAGS) $^ -o $@
$(BINDIR)/debugger: $(OBJDIR)/symbol_table.o $(OBJDIR)/memory.o $(OBJDIR)/register.o $(OBJDIR)/cpu.o $(OBJDIR)/utils.o $(OBJDIR)/darray.o $(OBJDIR)/decode_helper.o $(OBJDIR)/decode.o $(OBJDIR)/hashmap.o $(OBJDIR)/window.o $(OBJDIR)/debug_logic.o $(OBJDIR)/debugger.o
	$(CC) $(CFLAGS) $^ -o $@ -lncurses

#Creating object files
$(OBJDIR)/%.o:: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o:: $(SRCDIR)/$(EMUDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o:: $(SRCDIR)/$(ASMDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o:: $(SRCDIR)/$(EXTDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o:: $(SRCDIR)/$(ADTDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#Running Tests
test:
	$(MAKE) all
	cd $(TESTDIR); $(MAKE);
	cd $(TESTBINDIR); \
	for testbin in *; do \
		./$$testbin; \
	done

docs:
	$(MAKE) all
	cd $(LATEXDIR); $(MAKE);
	mkdir -p $(DOCOUTDIR);
	cp $(LATEXDIR)/refman.pdf $(DOCOUTDIR)/$(PDFNAME).pdf;

img:
	$(MAKE) all
	$(BINDIR)/assemble $(SRCDIR)/$(LEDBLINKDIR)/led_blink.s $(SRCDIR)/$(LEDBLINKDIR)/kernel8.img

clean:
	$(RM) $(BINS) $(OBJS)
	cd $(TESTDIR); $(MAKE) clean;
	cd $(DOCDIR); $(MAKE) cleanall;
