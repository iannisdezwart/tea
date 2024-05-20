#ifndef TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"

struct LiteralNumberExpression final : public ReadValue
{
	bool is_float;
	uint64_t value;

	LiteralNumberExpression(Token literal_number_token, const std::string &value)
		: ReadValue(std::move(literal_number_token), LITERAL_NUMBER_EXPRESSION),
		  is_float(value.find('.') != std::string::npos),
		  value(to_num(value, is_float)) {}

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
		std::string num;
		if (is_float)
		{
			double result = *reinterpret_cast<double *>(&value);
			num           = std::to_string(result);
		}
		else
		{
			num = std::to_string(value);
		}

		std::string s = "LiteralNumberExpression { value = \"" + num + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	static uint64_t
	to_num(const std::string &value, bool is_float)
	{
		if (is_float)
		{
			double result                 = std::stod(value);
			uint64_t result_reinterpreted = *reinterpret_cast<uint64_t *>(&result);
			return result_reinterpreted;
		}

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
		if (is_float)
		{
			type = Type(Type::FLOATING_POINT, 8);
			return;
		}

		if (value & 0xFF'00'00'00'00'00'00'00)
		{
			type = Type(Type::UNSIGNED_INTEGER, 8);
			return;
		}

		if (value & 0xFF'FF'00'00)
		{
			type = Type(Type::UNSIGNED_INTEGER, 4);
			return;
		}

		if (value & 0xFF'00)
		{
			type = Type(Type::UNSIGNED_INTEGER, 2);
			return;
		}

		type = Type(Type::UNSIGNED_INTEGER, 1);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		assembler.move_lit(value, result_reg);
	}
};

constexpr int LITERAL_NUMBER_EXPRESSION_SIZE = sizeof(LiteralNumberExpression);

#endif