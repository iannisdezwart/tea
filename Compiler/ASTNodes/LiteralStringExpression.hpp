#ifndef TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER
#define TEA_AST_NODE_LITERAL_STRING_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
literal_string_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
literal_string_expression_to_str(const AST &ast, uint node)
{
	std::string s = "LiteralStringExpression { value = \"";
	s += ast.strings[ast.data[node].literal_string_expression.string_id];
	s += "\" } @ ";
	s += std::to_string(node);
	return s;
}

void
literal_string_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint array_sizes_idx = ast.extra_data.size();
	ast.extra_data.push_back(1);
	ast.extra_data.push_back(0); // { 0 }
	ast.types[node] = Type(U8, 1, array_sizes_idx);
}

void
literal_string_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	std::string value      = ast.strings[ast.data[node].literal_string_expression.string_id];
	StaticData static_data = assembler.add_static_data(value);

	assembler.move_lit(static_data.offset, result_reg);
	assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
}

#endif