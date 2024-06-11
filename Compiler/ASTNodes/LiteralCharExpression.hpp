#ifndef TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_CHAR_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
literal_char_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
literal_char_expression_to_str(const AST &ast, uint node)
{
	std::string s = "LiteralCharExpression { value = ";
	s += std::to_string(ast.data[node].literal_char_expression.value);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

void
literal_char_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast.types[node] = Type(U8, 1);
}

void
literal_char_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assembler.move_lit(ast.data[node].literal_char_expression.value, result_reg);
}

#endif