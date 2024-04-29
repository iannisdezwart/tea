#ifndef TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct LiteralNumberExpression final : public ReadValue
{
	std::string value;

	LiteralNumberExpression(Token literal_number_token, std::string value)
		: ReadValue(std::move(literal_number_token), LITERAL_NUMBER_EXPRESSION),
		  value(std::move(value)) {}

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
		std::string s = "LiteralNumberExpression { value = \"" + value + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	uint64_t
	to_num()
		const
	{
		if (value[0] == '0' && value[1] == 'x')
			return std::stoull(value.substr(2), nullptr, 16);

		if (value[0] == '0' && value[1] == 'b')
			return std::stoull(value.substr(2), nullptr, 2);

		return std::stoull(value);
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type = Type(Type::UNSIGNED_INTEGER, 8);

		type.is_literal    = true;
		type.literal_value = &value;
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		assembler.move_64_into_reg(to_num(), result_reg);
	}
};

#endif