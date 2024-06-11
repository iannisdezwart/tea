#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
type_identifier_pair_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].type_identifier_pair.type_node, callback, depth + 1);
	callback(node, depth);
}

std::string
type_identifier_pair_to_str(const AST &ast, uint node)
{
	std::string s = "TypeIdentifierPair { identifier_id = \"";
	s += std::to_string(ast.data[node].type_identifier_pair.identifier_id);
	s += "\" } @ ";
	return s;
}

void
type_identifier_pair_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_name_node = ast.data[node].type_identifier_pair.type_node;
	ast_type_check(ast, type_name_node, type_check_state);
	ast.types[node] = ast.types[type_name_node];
}

#endif