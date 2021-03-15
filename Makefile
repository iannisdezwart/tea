all: VM/vm Assembler/assemble Compiler/compile

debug:
	g++ VM/vm.cpp -o VM/vm -g
	g++ Assembler/assemble.cpp -o Assembler/assemble -g
	g++ Compiler/main.cpp -o Compiler/compile -g

VM/vm: VM/vm.cpp
	g++ VM/vm.cpp -o VM/vm

Assembler/assemble: Assembler/assemble.cpp
	g++ Assembler/assemble.cpp -o Assembler/assemble

Compiler/compile: Compiler/main.cpp
	g++ Compiler/main.cpp -o Compiler/compile

clean:
	rm -rf VM/vm Assembler/assemble Compiler/compile