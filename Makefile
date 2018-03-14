# Makefile for building demo applications.

# Edit this file to compile extra C files into their own programs.
TARGETS= Ass2

CROSS_TOOL = 
CC_CPP = $(CROSS_TOOL)g++
CC_C = $(CROSS_TOOL)gcc

CFLAGS = -Wall -pthread -std=c99

all: clean $(TARGETS)

$(TARGETS):
	$(CC_C) thpool.c $(CFLAGS) $@.c -o $@

clean:
	rm -f $(TARGETS)