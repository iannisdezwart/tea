#ifndef TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER
#define TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class AssignmentExpression : public ASTNode {
	public:
		Token identifier_token;
		ASTNode *value;
		Token op_token;
		enum Operator op;

		AssignmentExpression(
			Token identifier_token,
			ASTNode *value,
			Token op_token
		) : identifier_token(identifier_token), value(value), op_token(op_token),
				ASTNode(op_token)
		{
			type = ASSIGNMENT_EXPRESSION;
			op = str_to_operator(op_token.value);

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			value->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "AssignmentExpression { op = \"" + to_string(op)
				+ "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			string id_name = identifier_token.value;
			Type id_type = compiler_state.get_type_of_identifier(id_name);
			Type value_type = value->get_type(compiler_state);

			if (id_type != value_type)
				err_at_token(identifier_token,
					"Type of identifier doesn't match type of its value",
					"id_type = %s, value_type = %s",
					id_type.to_str().c_str(), value_type.to_str().c_str());

			return id_type;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			// Moves result into R_ACCUMULATOR_0

			value->compile(assembler, compiler_state);

			string id_name = identifier_token.value;
			Variable& var = compiler_state.locals[id_name];
			Type& type = var.type;
			uint64_t offset = var.offset;
			uint64_t var_size = type.byte_size();

			// If we're doing direct assignment,
			// directly set the variable to the new value

			if (op == ASSIGNMENT) goto move_into_var;

			// We're doing some cool assignment,
			// move the result of the expression into R_ACCUMULATOR_1_ID

			assembler.move_reg_into_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);

			// Move the previous value into R_ACUMMULATOR_0_ID

			switch (var_size) {
				case 1:
					assembler.move_frame_offset_8_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 2:
					assembler.move_frame_offset_16_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 4:
					assembler.move_frame_offset_32_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 8:
					assembler.move_frame_offset_64_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;
			}

			// Perform the correct operation

			switch (op) {
				case SUM_ASSIGNMENT:
					assembler.add_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case DIFFERENCE_ASSIGNMENT:
					assembler.subtract_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case QUOTIENT_ASSIGNMENT:
					assembler.divide_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case REMAINDER_ASSIGNMENT:
					assembler.take_modulo_reg_of_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case PRODUCT_ASSIGNMENT:
					assembler.multiply_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case LEFT_SHIFT_ASSIGNMENT:
					assembler.left_shift_reg_by_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					break;

				case RIGHT_SHIFT_ASSIGNMENT:
					assembler.right_shift_reg_by_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					break;

				case BITWISE_AND_ASSIGNMENT:
					assembler.and_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_XOR_ASSIGNMENT:
					assembler.xor_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_OR_ASSIGNMENT:
					assembler.or_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;
			}

			move_into_var:

			// Move R_ACCUMULATOR_0 into the offset for the variable

			switch (var_size) {
				case 1:
					assembler.move_reg_into_frame_offset_8(R_ACCUMULATOR_0_ID, offset);
					break;

				case 2:
					assembler.move_reg_into_frame_offset_16(R_ACCUMULATOR_0_ID, offset);
					break;

				case 4:
					assembler.move_reg_into_frame_offset_32(R_ACCUMULATOR_0_ID, offset);
					break;

				case 8:
					assembler.move_reg_into_frame_offset_64(R_ACCUMULATOR_0_ID, offset);
					break;
			}
		}
};

#endif