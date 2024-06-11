#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
variable_declaration_uninitialised_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].variable_declaration_uninitialised.type_and_id_node, callback, depth + 1);
	callback(node, depth);
}

void
variable_declaration_initialised_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].variable_declaration_initialised.type_and_id_node, callback, depth + 1);

	uint ed_idx = ast.data[node].variable_declaration_initialised.ed_idx;

	uint id_expr_node = ast.extra_data[ed_idx];
	ast_dfs(ast, id_expr_node, callback, depth + 1);

	uint assignment_node = ast.extra_data[ed_idx + 1];
	ast_dfs(ast, assignment_node, callback, depth + 1);

	callback(node, depth);
}

std::string
variable_declaration_uninitialised_to_str(const AST &ast, uint node)
{
	return std::string("VariableDeclarationUninitialised {} @ ") + std::to_string(node);
}

std::string
variable_declaration_initialised_to_str(const AST &ast, uint node)
{
	return std::string("VariableDeclarationInitialised {} @ ") + std::to_string(node);
}

void
variable_declaration_uninitialised_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_and_id_node = ast.data[node].variable_declaration_uninitialised.type_and_id_node;
	ast_type_check(ast, type_and_id_node, type_check_state);

	ast.types[node] = ast.types[type_and_id_node];

	uint decl_id = ast.data[type_and_id_node].type_identifier_pair.identifier_id;

	if (!type_check_state.add_var(decl_id, ast.types[type_and_id_node], ast.extra_data))
	{
		err_at_token(ast.tokens[node],
			"Duplicate identifier name",
			"Identifier %d is already declared",
			ast.data[type_and_id_node].type_identifier_pair.identifier_id);
	}
}

void
variable_declaration_initialised_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_and_id_node = ast.data[node].variable_declaration_uninitialised.type_and_id_node;
	ast_type_check(ast, type_and_id_node, type_check_state);

	ast.types[node] = ast.types[type_and_id_node];

	uint decl_id = ast.data[type_and_id_node].type_identifier_pair.identifier_id;

	if (!type_check_state.add_var(decl_id, ast.types[type_and_id_node], ast.extra_data))
	{
		err_at_token(ast.tokens[node],
			"Duplicate identifier name",
			"Identifier %d is already declared",
			ast.data[type_and_id_node].type_identifier_pair.identifier_id);
	}

	uint ed_idx = ast.data[node].variable_declaration_initialised.ed_idx;

	uint id_expr_node = ast.extra_data[ed_idx];
	ast_type_check(ast, id_expr_node, type_check_state);

	uint assignment_node = ast.extra_data[ed_idx + 1];
	ast_type_check(ast, assignment_node, type_check_state);

	// Match types

	if (ast.types[assignment_node].fits(ast.types[node], ast.extra_data) == Type::Fits::NO)
	{
		warn("At %s, Initial value of VariableDeclaration does not"
		     "fit into specified type\n"
		     "lhs_type = %s, rhs_type = %s",
			ast.tokens[assignment_node].to_str().c_str(),
			ast.types[node].to_str(ast.extra_data).c_str(),
			ast.types[assignment_node].to_str(ast.extra_data).c_str());
	}
}

void
variable_declaration_initialised_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint8_t init_value_reg;

	if (ast.types[node].value >= BUILTIN_TYPE_END)
	{
		return;
	}

	// Array declaration

	if (ast.types[node].is_array(ast.extra_data))
	{
		return;
	}

	// Primitive type declaration
	// Get the expression value into a register and store it in memory

	uint ed_idx          = ast.data[node].variable_declaration_initialised.ed_idx;
	uint id_expr_node    = ast.extra_data[ed_idx];
	uint assignment_node = ast.extra_data[ed_idx + 1];

	init_value_reg = assembler.get_register();

	ast_get_value(ast, assignment_node, assembler, init_value_reg);
	ast_store(ast, id_expr_node, assembler, init_value_reg);

	assembler.free_register(init_value_reg);
}

#endif