# Makefile
# kpadron.github@gmail.com
# Kristian Padron
CC := gcc

CFLAGS := -Wall -Wextra
DEBUG := -g
OPT := -Og

INC := -I ./inc
vpath ./src

RM := -rm -f *.o *~ core

BINS := scanner

all: $(BINS)

r: clean all

scanner: main.o list.o hash.o
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $^

%: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -c -o $@ $^

clean:
	$(RM) $(BINS)
