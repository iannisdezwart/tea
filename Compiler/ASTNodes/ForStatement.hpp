#ifndef TEA_AST_NODE_FOR_STATEMENT_HEADER
#define TEA_AST_NODE_FOR_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "CodeBlock.hpp"

class ForStatement : public ASTNode {
	public:
		Token for_token;
		ASTNode *init;
		ReadValue *test;
		ReadValue *update;
		CodeBlock *body;

		ForStatement(ASTNode *init, ReadValue *test, ReadValue *update,
			Token for_token, CodeBlock *body)
				: init(init), test(test), update(update), body(body),
					for_token(for_token), ASTNode(for_token, FOR_STATEMENT) {}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			init->dfs(callback, depth + 1);
			test->dfs(callback, depth + 1);
			update->dfs(callback, depth + 1);
			body->dfs(callback, depth + 1);

			callback(this, depth);
		}

		std::string to_str()
		{
			std::string s = "ForStatement {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			uint8_t test_reg;

			// Create labels

			std::string loop_label = compiler_state.generate_label("for-loop");
			std::string end_label = compiler_state.generate_label("end-for-statement");

			// Compile code for the init statement

			init->compile(assembler, compiler_state);

			// Create the loop label

			assembler.add_label(loop_label);

			// Perform the check and move the result into the test register

			test_reg = assembler.get_register();
			test->get_value(assembler, compiler_state, test_reg);

			// Break the loop if false

			assembler.compare_reg_to_8(test_reg, 0);
			assembler.jump_if_equal(end_label);

			assembler.free_register(test_reg);

			// Compile code for the body block

			body->compile(assembler, compiler_state);

			// Compile code for the update expression

			update->compile(assembler, compiler_state);

			// Jump to the start of the loop again

			assembler.jump(loop_label);

			// Create the end label

			assembler.add_label(end_label);
		}
};

#endif