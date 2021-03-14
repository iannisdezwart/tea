all: VM/vm Assembler/assemble

debug:
	g++ VM/vm.cpp -o VM/vm -g
	g++ Assembler/assemble.cpp -o Assembler/assemble -g

VM/vm: VM/vm.cpp
	g++ VM/vm.cpp -o VM/vm

Assembler/assemble: Assembler/assemble.cpp
	g++ Assembler/assemble.cpp -o Assembler/assemble

clean:
	rm -rf VM/vm Assembler/assemble