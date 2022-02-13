all: VM/vm Disassembler/disassemble Compiler/compile Debugger/debug

doxygen: Doxyfile
	mkdir -p doxygen
	doxygen Doxyfile

debug:
	$(CXX) -std=c++17 VM/vm.cpp -o VM/vm -g
	$(CXX) -std=c++17 Disassembler/disassemble.cpp -o Disassembler/disassemble -g
	$(CXX) -std=c++17 Compiler/compile.cpp -o Compiler/compile -g
	$(CXX) -std=c++17 Debugger/debug.cpp -o Debugger/debug -g

VM/vm: VM/vm.cpp
	$(CXX) -std=c++17 VM/vm.cpp -o VM/vm -O3

Assembler/assemble: Assembler/assemble.cpp
	$(CXX) -std=c++17 Assembler/assemble.cpp -o Assembler/assemble -O3

Disassembler/disassemble: Disassembler/disassemble.cpp
	$(CXX) -std=c++17 Disassembler/disassemble.cpp -o Disassembler/disassemble -O3

Compiler/compile: Compiler/compile.cpp
	$(CXX) -std=c++17 Compiler/compile.cpp -o Compiler/compile -O3

Debugger/debug: Debugger/debug.cpp
	$(CXX) -std=c++17 Debugger/debug.cpp -o Debugger/debug -O3

clean:
	rm -rf VM/vm Disassembler/disassemble Assembler/assemble Compiler/compile Debugger/debug

format:
	clang-format -i **/*.cpp **/*.hpp