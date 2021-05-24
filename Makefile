CXX = g++

all: VM/vm Assembler/assemble Disassembler/disassemble Compiler/compile

debug:
	$(CXX) VM/vm.cpp -o VM/vm -g
	$(CXX) Assembler/assemble.cpp -o Assembler/assemble -g
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble -g
	$(CXX) Compiler/main.cpp -o Compiler/compile -g

VM/vm: VM/vm.cpp
	$(CXX) VM/vm.cpp -o VM/vm

Assembler/assemble: Assembler/assemble.cpp
	$(CXX) Assembler/assemble.cpp -o Assembler/assemble

Disassembler/disassemble: Disassembler/disassemble.cpp
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble

Compiler/compile: Compiler/main.cpp
	$(CXX) Compiler/main.cpp -o Compiler/compile

clean:
	rm -rf VM/vm Disassembler/disassemble Assembler/assemble Compiler/compile