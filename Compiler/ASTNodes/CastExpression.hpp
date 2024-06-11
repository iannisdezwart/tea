#ifndef TEA_AST_NODE_CAST_EXPRESSION_HEADER
#define TEA_AST_NODE_CAST_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
cast_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].cast_expression.type_name_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].cast_expression.expression_node, callback, depth + 1);
	callback(node, depth);
}

std::string
cast_expression_to_str(const AST &ast, uint node)
{
	return std::string("CastExpression {} @ ") + std::to_string(node);
}

void
cast_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast_type_check(ast, ast.data[node].cast_expression.type_name_node, type_check_state);
	ast_type_check(ast, ast.data[node].cast_expression.expression_node, type_check_state);

	ast.types[node] = ast.types[ast.data[node].cast_expression.type_name_node];
}

void
cast_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].cast_expression.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);
}

#endif