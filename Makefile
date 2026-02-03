.PHONY: all bareshell bytecode_vm clean

all: bareshell bytecode_vm

bareshell:
	$(MAKE) -C bareshell

bytecode_vm:
	$(MAKE) -C bytecode_vm all

clean:
	$(MAKE) -C bareshell clean
	$(MAKE) -C bytecode_vm clean

test_step: all
	cat tests/test_step.txt | ./bareshell/bareshell

test_breakpoints: all
	cat tests/test_breakpoints.txt | ./bareshell/bareshell

test_stack: all
	cat tests/test_stack.txt | ./bareshell/bareshell

test_mem: all
	cat tests/test_mem.txt | ./bareshell/bareshell

test_help: all
	cat tests/test_help.txt | ./bareshell/bareshell

test_heap_mem: all
	cat tests/test_heap_mem.txt | ./bareshell/bareshell

tests: test_step test_breakpoints test_stack test_mem test_help test_heap_mem
