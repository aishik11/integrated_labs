# Makefile for the assembler

CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

TARGET = bin/assembler

# Bison and Flex
BISON = bison
BISON_FLAGS = -d -o src/parser.tab.c
FLEX = flex
FLEX_FLAGS = -o src/lexer.yy.c

# Source files
SRC_DIR = src
LEXER_SRC = $(SRC_DIR)/lexer.l
PARSER_SRC = $(SRC_DIR)/parser.y

# Generated files
LEXER_GEN_SRC = $(patsubst %.l, %.yy.c, $(LEXER_SRC))
PARSER_GEN_HDR = $(patsubst %.y, %.tab.h, $(PARSER_SRC))
PARSER_GEN_SRC = $(patsubst %.y, %.tab.c, $(PARSER_SRC))

OBJS = $(patsubst %.c, %.o, $(LEXER_GEN_SRC) $(PARSER_GEN_SRC))

.PHONY: all clean test

all: $(TARGET)

test: all
	./tests/run_tests.sh

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PARSER_GEN_SRC) $(PARSER_GEN_HDR): $(PARSER_SRC)
	$(BISON) $(BISON_FLAGS) $(PARSER_SRC)

$(LEXER_GEN_SRC): $(LEXER_SRC) $(PARSER_GEN_HDR)
	$(FLEX) $(FLEX_FLAGS) $(LEXER_SRC)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(LEXER_GEN_SRC) $(PARSER_GEN_SRC) $(PARSER_GEN_HDR)