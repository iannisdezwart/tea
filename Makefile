all: VM/vm Disassembler/disassemble Compiler/compile Debugger/debug

doxygen: Doxyfile
	mkdir -p doxygen
	doxygen Doxyfile

COMMON_FLAGS = -std=c++17 -I./ -Wall
DEBUG = -g
FAST = -O3

debug:
	$(CXX) $(COMMON_FLAGS) VM/vm.cpp -o VM/vm $(DEBUG)
	$(CXX) $(COMMON_FLAGS) Disassembler/disassemble.cpp -o Disassembler/disassemble $(DEBUG)
	$(CXX) $(COMMON_FLAGS) Compiler/compile.cpp -o Compiler/compile $(DEBUG)
	$(CXX) $(COMMON_FLAGS) Debugger/debug.cpp -o Debugger/debug $(DEBUG)

VM/vm: VM/vm.cpp
	$(CXX) $(COMMON_FLAGS) VM/vm.cpp -o VM/vm $(FAST)

Assembler/assemble: Assembler/assemble.cpp
	$(CXX) $(COMMON_FLAGS) Assembler/assemble.cpp -o Assembler/assemble $(FAST)

Disassembler/disassemble: Disassembler/disassemble.cpp
	$(CXX) $(COMMON_FLAGS) Disassembler/disassemble.cpp -o Disassembler/disassemble $(FAST)

Compiler/compile: Compiler/compile.cpp
	$(CXX) $(COMMON_FLAGS) Compiler/compile.cpp -o Compiler/compile $(FAST)

Debugger/debug: Debugger/debug.cpp
	$(CXX) $(COMMON_FLAGS) Debugger/debug.cpp -o Debugger/debug $(FAST)

clean:
	rm -rf VM/vm Disassembler/disassemble Assembler/assemble Compiler/compile Debugger/debug

format:
	clang-format -i **/*.cpp **/*.hpp

test:
	./run-tests.sh