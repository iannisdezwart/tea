all: VM/vm Assembler/assemble Disassembler/disassemble Compiler/compile

debug:
	g++ VM/vm.cpp -o VM/vm -g
	g++ Assembler/assemble.cpp -o Assembler/assemble -g
	g++ Disassembler/disassemble.cpp -o Disassembler/disassemble -g
	g++ Compiler/main.cpp -o Compiler/compile -g

VM/vm: VM/vm.cpp
	g++ VM/vm.cpp -o VM/vm

Assembler/assemble: Assembler/assemble.cpp
	g++ Assembler/assemble.cpp -o Assembler/assemble

Disassembler/disassemble: Disassembler/disassemble.cpp
	g++ Disassembler/disassemble.cpp -o Disassembler/disassemble

Compiler/compile: Compiler/main.cpp
	g++ Compiler/main.cpp -o Compiler/compile

clean:
	rm -rf VM/vm Assembler/assemble Compiler/compile