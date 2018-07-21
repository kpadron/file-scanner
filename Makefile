# Makefile
# kpadron.github@gmail.com
# Kristian Padron
CC := gcc

CFLAGS := -Wall -Wextra
DEBUG := -g
OPT := -Og

INC := -I ./inc

RM := -rm -f *.o *~ core

BINS := scanner

all: $(BINS)

r: clean all

scanner: main.o hash.o
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $^ $(INC)

%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -c -o $@ $< $(INC)

%: %.c
	$(CC) $(CFLAGS) $(DEBUG) $(OPT) -o $@ $<

clean:
	$(RM) $(BINS)
