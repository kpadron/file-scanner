# Makefile
# kpadron.github@gmail.com
# Kristian Padron
CC := gcc

CFLAGS := -Wall -Wextra
DEBUG := -g
OPT :=

INC := -I ./inc

RM := -rm -f *.o *~ core

BINS := scanner

all: $(BINS)

test: hash_test

scanner: scanner.o hash.o
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $^ $(INC)

hash_test: hash_test.c hash.o
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $^ $(INC)

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -c -o $@ $< $(INC)

%: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $<

r: clean all

clean:
	$(RM) $(BINS)
