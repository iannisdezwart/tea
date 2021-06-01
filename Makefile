CXX = g++

all: VM/vm Assembler/assemble Disassembler/disassemble Compiler/compile Debugger/debug

debug:
	$(CXX) VM/vm.cpp -o VM/vm -g
	$(CXX) Assembler/assemble.cpp -o Assembler/assemble -g
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble -g
	$(CXX) Compiler/compile.cpp -o Compiler/compile -g
	$(CXX) Debugger/debug.cpp -o Debugger/debug -g

VM/vm: VM/vm.cpp
	$(CXX) VM/vm.cpp -o VM/vm

Assembler/assemble: Assembler/assemble.cpp
	$(CXX) Assembler/assemble.cpp -o Assembler/assemble

Disassembler/disassemble: Disassembler/disassemble.cpp
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble

Compiler/compile: Compiler/compile.cpp
	$(CXX) Compiler/compile.cpp -o Compiler/compile

Debugger/debug: Debugger/debug.cpp
	$(CXX) Debugger/debug.cpp -o Debugger/debug

clean:
	rm -rf VM/vm Disassembler/disassemble Assembler/assemble Compiler/compile Debugger/debug