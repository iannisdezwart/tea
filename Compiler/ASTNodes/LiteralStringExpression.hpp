#ifndef TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct LiteralStringExpression final : public ReadValue
{
	std::string value;

	LiteralStringExpression(Token literal_string_token, std::string value)
		: ReadValue(std::move(literal_string_token), LITERAL_STRING_EXPRESSION),
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
		std::string s = "LiteralStringExpression { value = \"" + value + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type = Type(Type::UNSIGNED_INTEGER, 1, { 0 });
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		StaticData static_data = assembler.add_static_data(value);

		assembler.move_lit(static_data.offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
	}
};

#endif