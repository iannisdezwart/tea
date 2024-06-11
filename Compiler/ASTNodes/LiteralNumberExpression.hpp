#ifndef TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_NUMBER_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
literal_number_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
literal_integer_expression_to_str(const AST &ast, uint node)
{
	std::string s = "LiteralIntegerExpression { value = ";
	s += std::to_string(ast.data[node].literal_integer_expression.value);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

std::string
literal_float_expression_to_str(const AST &ast, uint node)
{
	std::string s = "LiteralFloatExpression { value = ";
	s += std::to_string(ast.data[node].literal_float_expression.value);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

void
literal_integer_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint64_t value = ast.data[node].literal_integer_expression.value;

	if (value & 0xFF'00'00'00'00'00'00'00)
	{
		ast.types[node] = Type(U64, 8);
		return;
	}

	if (value & 0xFF'FF'00'00)
	{
		ast.types[node] = Type(U32, 4);
		return;
	}

	if (value & 0xFF'00)
	{
		ast.types[node] = Type(U16, 2);
		return;
	}

	ast.types[node] = Type(U8, 1);
}

void
literal_float_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast.types[node] = Type(F64, 8);
}

void
literal_integer_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assembler.move_lit(ast.data[node].literal_integer_expression.value, result_reg);
}

void
literal_float_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	// Reinterpret double value as integer.
	assembler.move_lit(ast.data[node].literal_integer_expression.value, result_reg);
}

#endif