#ifndef TEA_AST_NODES_BINARY_OPERATION_HEADER
#define TEA_AST_NODES_BINARY_OPERATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class BinaryOperation : public ASTNode {
	public:
		ASTNode *left;
		ASTNode *right;
		Token op_token;
		enum Operator op;

		BinaryOperation(ASTNode *left, ASTNode *right, Token op_token)
			: left(left), right(right), op_token(op_token), ASTNode(op_token)
		{
			type = BINARY_OPERATION;
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

			left->dfs(callback, depth + 1);
			right->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "BinaryOperation { op = \"" + to_string(op)
				+ "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type left_type = left->get_type(compiler_state);
			Type right_type = right->get_type(compiler_state);

			printf("left_type = %s, right_type = %s\n",
				left_type.to_str().c_str(), right_type.to_str().c_str());

			if (left_type != right_type) {
				printf("types don't match!!!\n");
			}

			return left_type;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			bool is_root = compiler_state.root_of_operation_tree;
			compiler_state.root_of_operation_tree = false;

			// Compile the left

			left->compile(assembler, compiler_state);

			if (!left->is_operation()) {
				assembler.push_reg_64(R_ACCUMULATOR_0_ID);
			}

			// Compile the right

			right->compile(assembler, compiler_state);

			if (!right->is_operation()) {
				assembler.push_reg_64(R_ACCUMULATOR_0_ID);
			}

			// Perform the operation

			assembler.pop_64_into_reg(R_ACCUMULATOR_1_ID);
			assembler.pop_64_into_reg(R_ACCUMULATOR_0_ID);

			switch (op) {
				case DIVISION:
					assembler.divide_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case REMAINDER:
					assembler.take_modulo_reg_of_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case MULTIPLICATION:
					assembler.multiply_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case ADDITION:
					assembler.add_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case SUBTRACTION:
					assembler.subtract_reg_from_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_AND:
					assembler.and_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_XOR:
					assembler.xor_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				case BITWISE_OR:
					assembler.or_reg_into_reg(R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
					break;

				default:
					printf("operator %d in BinaryOperation not implemented\n", op);
					abort();
					break;
			}

			// Push the result on the stack if this is not the root

			if (is_root) {
				compiler_state.root_of_operation_tree = true;
			} else {
				assembler.push_reg_64(R_ACCUMULATOR_0_ID);
			}

			get_type(compiler_state);
		}
};

#endif