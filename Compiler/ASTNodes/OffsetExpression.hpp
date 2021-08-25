#ifndef TEA_AST_NODE_OFFSET_EXPRESSION_HEADER
#define TEA_AST_NODE_OFFSET_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ReadValue.hpp"

using namespace std;

class OffsetExpression : public ReadValue {
	public:
		ReadValue *pointer;
		ReadValue *offset;

		OffsetExpression(ReadValue *pointer, ReadValue *offset, Token bracket_token)
			: pointer(pointer), offset(offset),
				ReadValue(bracket_token, OFFSET_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			pointer->dfs(callback, depth + 1);
			offset->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s;

			s += "OffsetExpression {} @ ";
			s += to_hex((size_t) this);

			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type offset_type = offset->get_type(compiler_state);

			if (offset_type.pointer_depth()) {
				err_at_token(accountable_token,
					"Type Error",
					"Offset provided in OffsetExpression is not an integer\n"
					"Found type %s instead",
					offset_type.to_str().c_str());
			}

			return pointer->get_type(compiler_state);
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state, uint8_t result_reg)
		{
			uint8_t offset_reg;

			Type pointer_type = get_type(compiler_state);

			// Multiply the offset by the byte size

			offset_reg = assembler.get_register();
			offset->get_value(assembler, compiler_state, offset_reg);
			assembler.multiply_8_into_reg(pointer_type.byte_size(), offset_reg);

			// Add the offset into the pointer

			pointer->get_value(assembler, compiler_state, result_reg);
			assembler.add_reg_into_reg(offset_reg, result_reg);

			assembler.free_register(offset_reg);

			// Dereference the pointer

			Type pointed_type = pointer_type.pointed_type();

			switch (pointed_type.byte_size()) {
				case 1:
					assembler.move_reg_pointer_8_into_reg(result_reg, result_reg);
					break;

				case 2:
					assembler.move_reg_pointer_16_into_reg(result_reg, result_reg);
					break;

				case 4:
					assembler.move_reg_pointer_32_into_reg(result_reg, result_reg);
					break;

				case 8:
					assembler.move_reg_pointer_64_into_reg(result_reg, result_reg);
					break;

				default:
					printf("Dereference assignment for "
						"	byte size %lu is not implemented\n", 
						pointed_type.byte_size());
					abort();
			}
		}
};

#endif