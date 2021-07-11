#ifndef TEA_AST_NODE_WHILE_STATEMENT_HEADER
#define TEA_AST_NODE_WHILE_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "CodeBlock.hpp"

using namespace std;

class WhileStatement : public ASTNode {
	public:
		Token while_token;
		ASTNode *test;
		CodeBlock *body;

		WhileStatement(ASTNode *test, Token while_token, CodeBlock *body)
			: test(test), body(body), while_token(while_token),
				ASTNode(while_token, WHILE_STATEMENT) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			test->dfs(callback, depth + 1);
			body->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "WhileStatement {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// Create labels

			string loop_label = compiler_state.generate_label("while-loop");
			string end_label = compiler_state.generate_label("end-while-statement");

			// Create the loop label

			assembler.add_label(loop_label);

			// Perform the check, moves the result into R_ACCUMULATOR_0

			test->compile(assembler, compiler_state);

			// Break the loop if false

			assembler.compare_reg_to_8(R_ACCUMULATOR_0_ID, 0);
			assembler.jump_if_equal(end_label);

			// Compile code for the body block

			body->compile(assembler, compiler_state);

			// Jump to the start of the loop again

			assembler.jump(loop_label);

			// Create the end label

			assembler.add_label(end_label);
		}
};

#endif