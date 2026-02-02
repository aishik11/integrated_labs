CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGET = debugger

ifeq ($(shell which bear 2>/dev/null),)
    BEAR_CMD :=
    $(warning "bear not found, compiling without it.")
else
    BEAR_CMD := bear --append --
endif

SRCS = $(shell find $(SRCDIR) -name '*.cpp')
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(BEAR_CMD) $(CXX) $(CXXFLAGS) -I$(INCDIR) -MMD -MP -c $< -o $@

.PHONY: clean run clean-compile-commands test_input_1 test_input_2 test_input_const test_input_basic tests

clean: clean-compile-commands
	rm -rf $(BUILDDIR) $(TARGET)

clean-compile-commands:
	rm -f compile_commands.json

run: $(TARGET)
	./$(TARGET)

test_input_1: $(TARGET)
	cat input_1.txt | ./$(TARGET) 

test_input_2: $(TARGET)
	cat input_2.txt | ./$(TARGET) 

test_input_const: $(TARGET)
	cat input_test_const.txt | ./$(TARGET) 

test_input_basic: $(TARGET)
	cat input.txt | ./$(TARGET) 

dummy/loop: dummy/loop.s
	as dummy/loop.s -o dummy/loop.o
	ld dummy/loop.o -o dummy/loop

test_loop: $(TARGET) dummy/loop
	cat input_loop.txt | ./$(TARGET)

test_loop_step: $(TARGET) dummy/loop
	cat input_loop_step.txt | ./$(TARGET)

tests_loop: test_loop test_loop_step

tests: test_input_1 test_input_2 test_input_const test_input_basic tests_loop

-include $(DEPS)
