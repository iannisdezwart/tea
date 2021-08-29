#ifndef TEA_AST_NODE_SYS_CALL_HEADER_HEADER
#define TEA_AST_NODE_SYS_CALL_HEADER_HEADER

#include <bits/stdc++.h>
#include "ASTNode.hpp"
#include "ReadValue.hpp"

set<string> syscall_names = { "PRINT_CHAR", "GET_CHAR" };

class SysCall : public ASTNode {
	public:
		Token name_token;
		vector<ReadValue *> arguments;

		SysCall(Token name_token, vector<ReadValue *>&& arguments)
			: name_token(name_token), arguments(std::move(arguments)),
				ASTNode(name_token, SYS_CALL) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			for (size_t i = 0; i < arguments.size(); i++) {
				arguments[i]->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		string to_str()
		{
			string s = "SysCall { name = \"" + name_token.value
				+ "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler &assembler, CompilerState& compiler_state)
		{
			if (name_token.value == "PRINT_CHAR") {
				if (arguments.size() != 1) {
					err_at_token(name_token, "Type Error",
						"Argument count in PRINT_CHAR SysCall is not equal to 1\n"
						"Expected a character as argument");
				}

				ReadValue *character = arguments[0];
				Type type = character->get_type(compiler_state);

				if (type.pointer_depth()) {
					err_at_token(name_token, "Type Error",
						"Argument in PRINT_CHAR SysCall is a pointer\n"
						"Expected a character as argument");
				}

				uint8_t char_reg = assembler.get_register();

				character->get_value(assembler, compiler_state, char_reg);
				assembler.print_char(char_reg);

				assembler.free_register(char_reg);
			}

			else if (name_token.value == "GET_CHAR") {
				if (arguments.size() != 1) {
					err_at_token(name_token, "Type Error",
						"Argument count in GET_CHAR SysCall is not equal to 1\n"
						"Expected a pointer to a character as argument");
				}

				ReadValue *char_pointer = arguments[0];
				Type type = char_pointer->get_type(compiler_state);

				if (type.pointer_depth() != 1 || type.pointed_type().byte_size() != 2) {
					err_at_token(name_token, "Type Error",
						"Argument in PRINT_CHAR SysCall is a non-pointer or a pointer to a pointer\n"
						"Expected a pointer to a 16-bit character as argument");
				}

				uint8_t char_reg = assembler.get_register();
				assembler.get_char(char_reg);

				uint8_t addr_reg = assembler.get_register();
				char_pointer->get_value(assembler, compiler_state, addr_reg);
				assembler.move_reg_into_reg_pointer_16(char_reg, addr_reg);

				assembler.free_register(char_reg);
				assembler.free_register(addr_reg);
			}
		}
};

#endif