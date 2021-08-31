#ifndef TEA_AST_NODE_OFFSET_EXPRESSION_HEADER
#define TEA_AST_NODE_OFFSET_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "ReadValue.hpp"

using namespace std;

class OffsetExpression : public WriteValue {
	public:
		WriteValue *pointer;
		ReadValue *offset;

		OffsetExpression(WriteValue *pointer, ReadValue *offset, Token bracket_token)
			: pointer(pointer), offset(offset),
				WriteValue(bracket_token, OFFSET_EXPRESSION) {}

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

			return pointer->get_type(compiler_state).pointed_type();
		}

		LocationData get_location_data(CompilerState& compiler_state)
		{
			return pointer->get_location_data(compiler_state);
		}

		void store(Assembler& assembler, CompilerState& compiler_state, uint8_t value_reg)
		{
			Type pointed_type = get_type(compiler_state);
			uint8_t offset_reg = assembler.get_register();
			LocationData location_data = get_location_data(compiler_state);

			// Local variable or parameter

			if (location_data.is_at_frame_top()) {
				switch (location_data.var_size) {
					case 1:
						offset->get_value(assembler, compiler_state, offset_reg);
						assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
						assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
						assembler.add_64_into_reg(location_data.offset, offset_reg);
						assembler.move_reg_into_reg_pointer_8(value_reg, offset_reg);
						break;

					case 2:
						offset->get_value(assembler, compiler_state, offset_reg);
						assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
						assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
						assembler.add_64_into_reg(location_data.offset, offset_reg);
						assembler.move_reg_into_reg_pointer_16(value_reg, offset_reg);
						break;

					case 4:
						offset->get_value(assembler, compiler_state, offset_reg);
						assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
						assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
						assembler.add_64_into_reg(location_data.offset, offset_reg);
						assembler.move_reg_into_reg_pointer_32(value_reg, offset_reg);
						break;

					case 8:
						offset->get_value(assembler, compiler_state, offset_reg);
						assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
						assembler.add_reg_into_reg(R_FRAME_PTR, offset_reg);
						assembler.add_64_into_reg(location_data.offset, offset_reg);
						assembler.move_reg_into_reg_pointer_64(value_reg, offset_reg);
						break;

					default:
						err_at_token(pointer->accountable_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}

				assembler.free_register(offset_reg);
				return;
			}

			// Global variable

			switch (location_data.var_size) {
				case 1:
					offset->get_value(assembler, compiler_state, offset_reg);
					assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
					assembler.move_stack_top_address_into_reg(offset_reg);
					assembler.add_64_into_reg(location_data.offset, offset_reg);
					assembler.move_reg_into_reg_pointer_8(value_reg, offset_reg);
					break;

				case 2:
					offset->get_value(assembler, compiler_state, offset_reg);
					assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
					assembler.move_stack_top_address_into_reg(offset_reg);
					assembler.add_64_into_reg(location_data.offset, offset_reg);
					assembler.move_reg_into_reg_pointer_16(value_reg, offset_reg);
					break;

				case 4:
					offset->get_value(assembler, compiler_state, offset_reg);
					assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
					assembler.move_stack_top_address_into_reg(offset_reg);
					assembler.add_64_into_reg(location_data.offset, offset_reg);
					assembler.move_reg_into_reg_pointer_32(value_reg, offset_reg);
					break;

				case 8:
					offset->get_value(assembler, compiler_state, offset_reg);
					assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);
					assembler.move_stack_top_address_into_reg(offset_reg);
					assembler.add_64_into_reg(location_data.offset, offset_reg);
					assembler.move_reg_into_reg_pointer_64(value_reg, offset_reg);
					break;

				default:
					err_at_token(pointer->accountable_token,
						"Type Error",
						"Variable doesn't fit in register\n"
						"Support for this is not implemented yet");
			}

			assembler.free_register(offset_reg);
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state, uint8_t result_reg)
		{
			uint8_t offset_reg;

			Type pointed_type = get_type(compiler_state);

			// Multiply the offset by the byte size

			offset_reg = assembler.get_register();
			offset->get_value(assembler, compiler_state, offset_reg);
			assembler.multiply_8_into_reg(pointed_type.byte_size(), offset_reg);

			// Add the offset into the pointer

			pointer->get_value(assembler, compiler_state, result_reg);
			assembler.add_reg_into_reg(offset_reg, result_reg);

			assembler.free_register(offset_reg);

			// Dereference the pointer

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