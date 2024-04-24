#ifndef TEA_AST_NODE_IF_STATEMENT_HEADER
#define TEA_AST_NODE_IF_STATEMENT_HEADER

#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "CodeBlock.hpp"

struct IfStatement : public ASTNode
{
	Token if_token;
	ReadValue *test;
	CodeBlock *then_block;
	CodeBlock *else_block;

	IfStatement(ReadValue *test, Token if_token, CodeBlock *then_block,
		CodeBlock *else_block)
		: test(test), then_block(then_block), else_block(else_block),
		  if_token(if_token), ASTNode(if_token, IF_STATEMENT) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		test->dfs(callback, depth + 1);
		then_block->dfs(callback, depth + 1);
		if (else_block != NULL)
			else_block->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "IfStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		return Type();
	}

	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
		uint8_t test_reg;

		// Create labels

		std::string else_label = compiler_state.generate_label("else-block");
		std::string end_label  = compiler_state.generate_label("end-if-statement");

		// Perform the check and move the result into the test register

		test_reg = assembler.get_register();
		test->get_value(assembler, compiler_state, test_reg);

		// Jump to the right label

		assembler.compare_reg_to_8(test_reg, 0);
		assembler.jump_if_equal(else_label);

		assembler.free_register(test_reg);

		// Compile code for then block

		then_block->compile(assembler, compiler_state);
		assembler.jump(end_label);

		// Compile code for else block

		assembler.add_label(else_label);
		if (else_block != NULL)
			else_block->compile(assembler, compiler_state);

		// Create the end label

		assembler.add_label(end_label);
	}
};

#endif