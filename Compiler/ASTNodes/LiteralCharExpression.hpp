#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct LiteralCharExpression final : public ReadValue
{
	uint8_t value;

	LiteralCharExpression(CompactToken accountable_token, uint8_t value)
		: ReadValue(std::move(accountable_token), LITERAL_CHAR_EXPRESSION),
		  value(value) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "LiteralCharExpression { value = \"";
		s += std::to_string(value);
		s += "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type = Type(Type::UNSIGNED_INTEGER, 1);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		assembler.move_lit(value, result_reg);
	}
};

#endif