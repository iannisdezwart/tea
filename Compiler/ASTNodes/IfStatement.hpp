#ifndef TEA_AST_NODE_IF_STATEMENT_HEADER
#define TEA_AST_NODE_IF_STATEMENT_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "CodeBlock.hpp"

using namespace std;

class IfStatement : public ASTNode {
	public:
		Token if_token;
		ASTNode *test;
		CodeBlock *then_block;
		CodeBlock *else_block;

		IfStatement(ASTNode *test, Token if_token, CodeBlock *then_block,
			CodeBlock *else_block)
				: test(test), then_block(then_block), else_block(else_block),
					if_token(if_token), ASTNode(if_token, IF_STATEMENT) {}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			test->dfs(callback, depth + 1);
			then_block->dfs(callback, depth + 1);
			if (else_block != NULL) else_block->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "IfStatement {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			// Create labels

			string else_label = compiler_state.generate_label("else-block");
			string end_label = compiler_state.generate_label("end-if-statement");

			// Perform the check, moves the result into R_ACCUMULATOR_0

			test->compile(assembler, compiler_state);

			// Jump to the right label

			assembler.compare_reg_to_8(R_ACCUMULATOR_0_ID, 0);
			assembler.jump_if_equal(else_label);

			// Compile code for then block

			then_block->compile(assembler, compiler_state);
			assembler.jump(end_label);

			// Compile code for else block

			assembler.add_label(else_label);
			if (else_block != NULL) else_block->compile(assembler, compiler_state);

			// Create the end label

			assembler.add_label(end_label);
		}
};

#endif