# Makefile
# kpadron.github@gmail.com
# Kristian Padron
CC := gcc

CFLAGS := -Wall -Wextra
DEBUG := -g -Og

INC := -I ./inc

RM := -rm -f *.o *~ core

BINS := scanner

all: $(BINS)

test: hash_test

scanner: main.o scanner.o stack.o
	$(CC) $(CFLAGS) $(DEBUG) -o $@ $^ $(INC)

hash_test: hash_test.c hash.o
	$(CC) $(CFLAGS) $(DEBUG) -o $@ $^ $(INC)

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) -c -o $@ $< $(INC)

%: %.c
	$(CC) $(CFLAGS) $(DEBUG) -o $@ $<

r: clean all

clean:
	$(RM) $(BINS)
