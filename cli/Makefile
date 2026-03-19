# Makefile for zaster CLI binary
#
# Usage:
#   make                                    # build with defaults
#   make CC=clang LD=clang CFLAGS="-O3" LDFLAGS="-lpthread"
#   make clean
#
# All compiler/linker settings can be overridden for cross-compilation.
# LIBS_DIR should point to the directory containing <target>/libzstd.a and <target>/libzaster.a.

CC      ?= cc
LD      ?= $(CC)
CFLAGS  ?= -O3 -DNDEBUG
LDFLAGS ?=

# Source files (all .c in this directory)
SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
TARGET  := zaster

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
