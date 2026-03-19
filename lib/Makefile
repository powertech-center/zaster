# Makefile for libzaster.a
#
# Usage:
#   make                                    # build with defaults
#   make CC=clang-aarch64-linux-gnu AR=llvm-ar CFLAGS="-O3 -fPIC"
#   make clean
#
# All compiler/linker/archiver settings can be overridden for cross-compilation.

CC      ?= cc
AR      ?= ar
CFLAGS  ?= -O3 -DNDEBUG
ARFLAGS ?= rcs

# Source files (all .c in this directory)
SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
TARGET  := libzaster.a

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
