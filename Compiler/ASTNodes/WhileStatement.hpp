#ifndef TEA_AST_NODE_WHILE_STATEMENT_HEADER
#define TEA_AST_NODE_WHILE_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"

struct WhileStatement final : public ASTNode
{
	std::unique_ptr<ReadValue> test;
	std::unique_ptr<CodeBlock> body;

	WhileStatement(std::unique_ptr<ReadValue> test, Token while_token, std::unique_ptr<CodeBlock> body)
		: ASTNode(std::move(while_token), WHILE_STATEMENT),
		  test(std::move(test)),
		  body(std::move(body)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		test->dfs(callback, depth + 1);
		body->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "WhileStatement {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type_check_state.begin_local_scope();

		test->type_check(type_check_state);
		body->type_check(type_check_state);

		type_check_state.end_local_scope();
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t test_reg;

		// Create labels

		auto [start_label, end_label] = assembler.push_loop_scope();

		// Create the loop label

		assembler.add_label(start_label);

		// Perform the check and move the result into the test register

		test_reg = assembler.get_register();
		test->get_value(assembler, test_reg);

		// Break the loop if false

		uint8_t cmp_reg = assembler.get_register();
		assembler.move_lit(0, cmp_reg);
		assembler.cmp_int_8(test_reg, cmp_reg);
		assembler.free_register(cmp_reg);
		assembler.jump_if_eq(end_label);

		assembler.free_register(test_reg);

		// Compile code for the body block

		body->code_gen(assembler);

		// Jump to the start of the loop again

		assembler.jump(start_label);

		// Create the end label

		assembler.add_label(end_label);

		assembler.pop_loop_scope();
	}
};

constexpr int WHILE_STATEMENT_SIZE = sizeof(WhileStatement);

#endif