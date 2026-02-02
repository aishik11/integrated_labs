.PHONY: all bareshell bytecode_vm clean

all: bareshell bytecode_vm

bareshell:
	$(MAKE) -C bareshell

bytecode_vm:
	$(MAKE) -C bytecode_vm all

clean:
	$(MAKE) -C bareshell clean
	$(MAKE) -C bytecode_vm clean
