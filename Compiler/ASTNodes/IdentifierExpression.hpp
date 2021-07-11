#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "WriteValue.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"

using namespace std;

class IdentifierExpression : public WriteValue {
	public:
		Token identifier_token;
		WriteValue *replacement = NULL;
		Type replacement_type;

		IdentifierExpression(Token identifier_token)
			: identifier_token(identifier_token),
				WriteValue(identifier_token, IDENTIFIER_EXPRESSION) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			callback(this, depth);
		}

		string to_str()
		{
			string s = "IdentifierExpression { identifier = \""
				+ identifier_token.value + "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			if (replacement != NULL) {
				return replacement_type;
			}

			string id_name = identifier_token.value;

			Type type = compiler_state.get_type_of_identifier(id_name);

			if (type == Type::UNDEFINED)
				err_at_token(identifier_token,
					"Identifier has unknown kind",
					"Identifier: %s. this might be a bug in the compiler",
					id_name.c_str());

			return type;
		}

		LocationData get_location_data(CompilerState& compiler_state)
		{
			string id_name = identifier_token.value;
			IdentifierKind id_kind = compiler_state.get_identifier_kind(id_name);

			if (id_kind == IdentifierKind::UNDEFINED)
				err_at_token(identifier_token,
					"Reference to undeclared variable",
					"Identifier %s was referenced, but not declared",
					id_name.c_str());

			int64_t offset;
			uint64_t var_size;

			switch (id_kind) {
				case IdentifierKind::LOCAL:
				{
					Variable& var = compiler_state.locals[id_name];
					Type& type = var.id.type;
					offset = var.offset;
					var_size = type.byte_size();
					break;
				}

				case IdentifierKind::PARAMETER:
				{
					Variable& var = compiler_state.parameters[id_name];
					Type& type = var.id.type;
					offset = -compiler_state.parameters_size + var.offset
						- 8 - CPU::stack_frame_size;
					var_size = type.byte_size();
					break;
				}

				case IdentifierKind::GLOBAL:
				{
					Variable& var = compiler_state.globals[id_name];
					Type& type = var.id.type;
					offset = var.offset;
					var_size = type.byte_size();
					break;
				}

				default:
					err_at_token(identifier_token,
						"Identifier has unknown kind",
						"Identifier: %s. this might be a bug in the compiler",
						id_name.c_str());
			}

			return LocationData(id_kind, offset, var_size);
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state)
		{
			if (replacement != NULL) {
				replacement->get_value(assembler, compiler_state);
				return;
			}

			LocationData location_data = get_location_data(compiler_state);

			// Local variable or parameter

			if (location_data.is_at_frame_top()) {
				switch (location_data.var_size) {
					case 1:
						assembler.move_frame_offset_8_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 2:
						assembler.move_frame_offset_16_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 4:
						assembler.move_frame_offset_32_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 8:
						assembler.move_frame_offset_64_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					default:
						err_at_token(identifier_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}

				return;
			}

			// Global variable

			switch (location_data.var_size) {
				case 1:
					assembler.move_stack_top_offset_8_into_reg(
						location_data.offset, R_ACCUMULATOR_0_ID);
					break;

				case 2:
					assembler.move_stack_top_offset_16_into_reg(
						location_data.offset, R_ACCUMULATOR_0_ID);
					break;

				case 4:
					assembler.move_stack_top_offset_32_into_reg(
						location_data.offset, R_ACCUMULATOR_0_ID);
					break;

				case 8:
					assembler.move_stack_top_offset_64_into_reg(
						location_data.offset, R_ACCUMULATOR_0_ID);
					break;

				default:
					err_at_token(identifier_token,
						"Type Error",
						"Variable doesn't fit in register\n"
						"Support for this is not implemented yet");
			}
		}

		void store(Assembler& assembler, CompilerState& compiler_state)
		{
			if (replacement != NULL) {
				replacement->store(assembler, compiler_state);
				return;
			}

			LocationData location_data = get_location_data(compiler_state);

			// Local variable or parameter

			if (location_data.is_at_frame_top()) {
				switch (location_data.var_size) {
					case 1:
						assembler.move_reg_into_frame_offset_8(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 2:
						assembler.move_reg_into_frame_offset_16(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 4:
						assembler.move_reg_into_frame_offset_32(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 8:
						assembler.move_reg_into_frame_offset_64(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					default:
						err_at_token(identifier_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}

				return;
			}

			// Global variable

			switch (location_data.var_size) {
				case 1:
					assembler.move_reg_into_stack_top_offset_8(
						R_ACCUMULATOR_0_ID, location_data.offset);
					break;

				case 2:
					assembler.move_reg_into_stack_top_offset_16(
						R_ACCUMULATOR_0_ID, location_data.offset);
					break;

				case 4:
					assembler.move_reg_into_stack_top_offset_32(
						R_ACCUMULATOR_0_ID, location_data.offset);
					break;

				case 8:
					assembler.move_reg_into_stack_top_offset_64(
						R_ACCUMULATOR_0_ID, location_data.offset);
					break;

				default:
					err_at_token(identifier_token,
						"Type Error",
						"Variable doesn't fit in register\n"
						"Support for this is not implemented yet");
			}
		}
};

#endif