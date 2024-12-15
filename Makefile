# Makefile for building test program with system malloc and custom malloc

CC = gcc
CFLAGS = -Wall -g

# Default target: build both versions
all: test_orig test_new

# Build test program using the system malloc functions
test_orig: test.c
	$(CC) $(CFLAGS) test.c -o test_orig.exe

# Build test program using the custom malloc functions
test_new: test.c myMalloc.c
	$(CC) $(CFLAGS) test.c myMalloc.c -o test_new.exe -DUSE_MY_MALLOC

# Clean up generated files
clean:
	rm -f *.o *.exe
