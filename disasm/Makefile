#
# Makefile for Ninja Assembler
#

CC = gcc
CFLAGS = -Wall -Wno-unused -g
LDFLAGS = -g
LDLIBS = -lm

SRCS = disasm.c
OBJS = $(patsubst %.c,%.o,$(SRCS))
BIN = disasm

.PHONY:		all clean

all:		$(BIN)

$(BIN):		$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o:		%.c
		$(CC) $(CFLAGS) -o $@ -c $<

clean:
		rm -f *~ *.o $(BIN)
