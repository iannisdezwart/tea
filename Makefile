all: VM/vm Disassembler/disassemble Compiler/compile Debugger/debug

doxygen: Doxyfile
	mkdir -p doxygen
	doxygen Doxyfile

debug:
	$(CXX) VM/vm.cpp -o VM/vm -g
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble -g
	$(CXX) Compiler/compile.cpp -o Compiler/compile -g
	$(CXX) Debugger/debug.cpp -o Debugger/debug -g

VM/vm: VM/vm.cpp
	$(CXX) VM/vm.cpp -o VM/vm -O3

Assembler/assemble: Assembler/assemble.cpp
	$(CXX) Assembler/assemble.cpp -o Assembler/assemble -O3

Disassembler/disassemble: Disassembler/disassemble.cpp
	$(CXX) Disassembler/disassemble.cpp -o Disassembler/disassemble -O3

Compiler/compile: Compiler/compile.cpp
	$(CXX) Compiler/compile.cpp -o Compiler/compile -O3

Debugger/debug: Debugger/debug.cpp
	$(CXX) Debugger/debug.cpp -o Debugger/debug -O3

clean:
	rm -rf VM/vm Disassembler/disassemble Assembler/assemble Compiler/compile Debugger/debug

format:
	find . -iname *.hpp -o -iname *.cpp | xargs clang-format -i