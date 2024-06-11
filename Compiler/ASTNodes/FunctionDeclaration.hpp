#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/VariableDeclaration.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
function_declaration_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	type_identifier_pair_dfs(ast, ast.data[node].function_declaration.type_and_id_node, callback, depth + 1);

	uint ed_idx     = ast.data[node].function_declaration.ed_idx;
	uint params_len = ast.extra_data[ed_idx + 2];
	for (uint i = ed_idx + 3; i < ed_idx + 3 + params_len; i++)
	{
		uint param_node = ast.extra_data[i];
		ast_dfs(ast, param_node, callback, depth + 1);
	}

	uint body_node = ast.extra_data[ed_idx];
	ast_dfs(ast, body_node, callback, depth + 1);
}

std::string
function_declaration_to_str(const AST &ast, uint node)
{
	return std::string("FunctionDeclaration {} @ ") + std::to_string(node);
}

void
function_declaration_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_and_id_node = ast.data[node].function_declaration.type_and_id_node;
	type_identifier_pair_type_check(ast, type_and_id_node, type_check_state);

	Type return_type = ast.types[type_and_id_node];
	FunctionSignature fn_signature(ast.data[type_and_id_node].type_identifier_pair.identifier_id, return_type);

	// Add parameters

	uint ed_idx     = ast.data[node].function_declaration.ed_idx;
	uint params_len = ast.extra_data[ed_idx + 2];
	for (uint i = ed_idx + 3; i < ed_idx + 3 + params_len; i++)
	{
		uint param_node = ast.extra_data[i];
		ast_type_check(ast, param_node, type_check_state);

		uint param_id = ast.data[param_node].type_identifier_pair.identifier_id;
		fn_signature.parameters.push_back(IdentifierDefinition(param_id, ast.types[param_node]));
	}

	uint fn_id = ast.data[type_and_id_node].type_identifier_pair.identifier_id;
	if (!type_check_state.add_function(fn_id, fn_signature))
	{
		err_at_token(ast.tokens[node],
			"Duplicate identifier name",
			"Identifier %d is already declared",
			fn_id);
	}
}

void
function_declaration_type_check_body(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint ed_idx     = ast.data[node].function_declaration.ed_idx;
	uint params_len = ast.extra_data[ed_idx + 2];
	for (uint i = ed_idx + 3; i < ed_idx + 3 + params_len; i++)
	{
		uint param_node = ast.extra_data[i];
		uint param_id   = ast.data[param_node].type_identifier_pair.identifier_id;
		type_check_state.add_parameter(param_id, ast.types[param_node], ast.extra_data);
	}

	// Type check the function body

	uint type_and_id_node = ast.data[node].function_declaration.type_and_id_node;
	uint fn_id            = ast.data[type_and_id_node].type_identifier_pair.identifier_id;
	type_check_state.begin_function_scope(fn_id);

	uint body_node = ast.extra_data[ed_idx];
	ast_type_check(ast, body_node, type_check_state);

	ast.extra_data[ed_idx + 1] = type_check_state.locals_size;
	type_check_state.end_function_scope(ast.extra_data);
}

void
function_declaration_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint type_id_pair_node = ast.data[node].function_declaration.type_and_id_node;
	uint fn_id             = ast.data[type_id_pair_node].type_identifier_pair.identifier_id;

	assembler.add_label(fn_id);
	if (assembler.debug)
	{
		assembler.label(fn_id);
	}

	// Make space for the locals on the stack

	uint ed_idx = ast.data[node].function_declaration.ed_idx;
	uint locals_size = ast.extra_data[ed_idx + 1];
	if (locals_size > 0)
	{
		assembler.allocate_stack(locals_size);
	}

	// Compile the function body

	uint body_node = ast.extra_data[ed_idx];
	ast_code_gen(ast, body_node, assembler);

	assembler.return_();
}

#endif