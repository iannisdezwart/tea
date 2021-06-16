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

		bool warned = false;

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

			size_t left_size = left_type.byte_size();
			size_t right_size = right_type.byte_size();

			#define not_implemented_warning(op) do { \
				if (!warned) { \
					warned = true; \
					warn( \
						"operator %s (%s) is not implemented for types x = %s and y = %s " \
						"and might cause undefined behaviour\n" \
						"At %ld:%ld\n", \
						op_to_str(op), op_to_example_str(op), left_type.to_str().c_str(), \
						right_type.to_str().c_str(), op_token.line, op_token.col); \
				} \
			} while (0)

			switch (op) {
				case ADDITION:
				case SUBTRACTION:
				{
					// intX + intY -> max(intX, intY)

					if (left_type.pointer_depth == 0 && right_type.pointer_depth == 0) {
						if (left_size > right_size) {
							return left_type;
						}

						return right_type;
					}

					// pointer + int -> pointer

					if (left_type.pointer_depth > 0 && right_type.pointer_depth == 0) {
						return left_type;
					}

					// int + pointer -> pointer

					if (right_type.pointer_depth > 0 && left_type.pointer_depth == 0) {
						return right_type;
					}

					break;
				}

				case MULTIPLICATION:
				{
					// intX * intY -> max(intX, intY)

					if (left_type.pointer_depth == 0 && right_type.pointer_depth == 0) {
						if (left_size > right_size) {
							return left_type;
						}

						return right_type;
					}

					break;
				}

				case DIVISION:
				{
					// intX / intY -> intX

					if (left_type.pointer_depth == 0 && right_type.pointer_depth == 0) {
						return left_type;
					}

					break;
				}

				case REMAINDER:
				{
					// intX % intY -> intX

					if (left_type.pointer_depth == 0 && right_type.pointer_depth == 0) {
						return left_type;
					}
				}

				case BITWISE_AND:
				case BITWISE_XOR:
				case BITWISE_OR:
				{
					// intX & intX

					if (
						left_type.pointer_depth == 0 && right_type.pointer_depth == 0
						&& left_size == right_size
					) {
						return left_type;
					}
				}

				case LESS:
				case LESS_OR_EQUAL:
				case GREATER:
				case GREATER_OR_EQUAL:
				case EQUAL:
				case NOT_EQUAL:
				{
					return Type(Type::SIGNED_INTEGER, 1, 0);
				}

				default:
					printf("operator %s in BinaryOperation not implemented\n", op_to_str(op));
					abort();
					break;
			}

			not_implemented_warning(op);
			return left_type;

			#undef not_implemented_warning
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
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
				// Mathematical binary operations

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

				// Logical binary operations

				case LESS:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_less(R_ACCUMULATOR_0_ID);
					break;

				case LESS_OR_EQUAL:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_less_or_equal(R_ACCUMULATOR_0_ID);
					break;

				case GREATER:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_greater(R_ACCUMULATOR_0_ID);
					break;

				case GREATER_OR_EQUAL:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_greater_or_equal(R_ACCUMULATOR_0_ID);
					break;

				case EQUAL:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_equal(R_ACCUMULATOR_0_ID);
					break;

				case NOT_EQUAL:
					assembler.compare_reg_to_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);
					assembler.set_reg_if_not_equal(R_ACCUMULATOR_0_ID);
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