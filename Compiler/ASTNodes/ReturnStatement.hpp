#ifndef TEA_AST_NODE_RETURN_STATEMENT_HEADER
#define TEA_AST_NODE_RETURN_STATEMENT_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
return_void_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

void
return_expression_statement_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].return_expression_statement.expression_node, callback, depth + 1);
	callback(node, depth);
}

std::string
return_void_statement_to_str(const AST &ast, uint node)
{
	return std::string("ReturnVoidStatement {} @ ") + std::to_string(node);
}

std::string
return_expression_statement_to_str(const AST &ast, uint node)
{
	return std::string("ReturnExpressionStatement {} @ ") + std::to_string(node);
}

void
return_void_statement_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast.types[node] = Type(V0, 0);
}

void
return_expression_statement_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint expression_node = ast.data[node].return_expression_statement.expression_node;
	ast_type_check(ast, expression_node, type_check_state);
	ast.types[node] = ast.types[expression_node];
}

void
return_void_statement_code_gen(Assembler &assembler)
{
	assembler.return_();
}

void
return_expression_statement_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint8_t res_reg = assembler.get_register();

	// Store value in result register.

	uint expr_node = ast.data[node].return_expression_statement.expression_node;
	ast_get_value(ast, expr_node, assembler, res_reg);

	// Return value.

	assembler.move(res_reg, R_RET);
	assembler.free_register(res_reg);
	assembler.return_();
}

#endif