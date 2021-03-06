#
# Makefile for Ninja Compiler
#

CC = gcc
CFLAGS = -Wall -ansi -pedantic -g
LDFLAGS = -g
LDLIBS = -lm

SRCS = main.c utils.c parser.tab.c lex.yy.c sym.c \
       absyn.c semant.c table.c types.c codegen.c \
       vmt.c instance.c

OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = njc
#DIRS = nja njvm disasm

.PHONY:		all tests clean

all:		$(BIN)
#		-for d in $(DIRS); do (cd $$d; $(MAKE) ); done

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

parser.tab.h:	parser.y
		bison -d parser.y

parser.tab.c:	parser.y
		bison -d parser.y

lex.yy.c:	scanner.l parser.tab.h
		flex scanner.l

tests:		$(BIN)
		@for i in tests/test??.nj ; do \
		  echo ; \
		  echo -- Compiling $$i -- ; \
		  awk '{if(FNR==2)print}' $$i ; \
		  ./$(BIN) --absyn --output tests/`basename $$i .nj`.asm $$i ; \
		done
		@echo

-include depend.mak

depend.mak:	parser.tab.c lex.yy.c
		$(CC) $(CFLAGS) -MM $(SRCS) > depend.mak

clean:
		rm -f *~ *.o $(BIN)
		rm -f parser.tab.h parser.tab.c lex.yy.c depend.mak
		rm -f tests/*~ tests/*.asm
		rm -f test_fm/*.out test_fm/*.tmp
#		-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

test:
		./autotest.pl
