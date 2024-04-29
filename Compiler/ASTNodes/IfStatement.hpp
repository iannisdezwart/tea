#ifndef TEA_AST_NODE_IF_STATEMENT_HEADER
#define TEA_AST_NODE_IF_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"

struct IfStatement final : public ASTNode
{
	std::unique_ptr<ReadValue> test;
	std::unique_ptr<CodeBlock> then_block;
	std::unique_ptr<CodeBlock> else_block;

	IfStatement(std::unique_ptr<ReadValue> test, Token if_token,
		std::unique_ptr<CodeBlock> then_block, std::unique_ptr<CodeBlock> else_block)
		: ASTNode(std::move(if_token), IF_STATEMENT),
		  test(std::move(test)),
		  then_block(std::move(then_block)),
		  else_block(std::move(else_block)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		test->dfs(callback, depth + 1);
		then_block->dfs(callback, depth + 1);
		if (else_block)
			else_block->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "IfStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		test->type_check(type_check_state);
		then_block->type_check(type_check_state);

		if (else_block)
			else_block->type_check(type_check_state);
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t test_reg;

		// Create labels

		std::string else_label = assembler.generate_label("else-block");
		std::string end_label  = assembler.generate_label("end-if-statement");

		// Perform the check and move the result into the test register

		test_reg = assembler.get_register();
		test->get_value(assembler, test_reg);

		// Jump to the right label

		assembler.compare_reg_to_8(test_reg, 0);
		assembler.jump_if_equal(else_label);

		assembler.free_register(test_reg);

		// Compile code for then block

		then_block->code_gen(assembler);
		assembler.jump(end_label);

		// Compile code for else block

		assembler.add_label(else_label);
		if (else_block)
			else_block->code_gen(assembler);

		// Create the end label

		assembler.add_label(end_label);
	}
};

#endif