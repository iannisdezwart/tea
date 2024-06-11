#ifndef TEA_AST_NODE_CLASS_DECLARATION_HEADER
#define TEA_AST_NODE_CLASS_DECLARATION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/FunctionDeclaration.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
class_declaration_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	uint field_idx = ast.data[node].class_declaration.fields_ed_idx;
	uint len       = ast.extra_data[field_idx];
	for (uint i = field_idx + 1; i < field_idx + 1 + len; i++)
	{
		uint field_node = ast.extra_data[i];
		ast_dfs(ast, field_node, callback, depth + 1);
	}

	callback(node, depth);
}

std::string
class_declaration_to_str(const AST &ast, uint node)
{
	std::string s = "ClassDeclaration { class_id = ";
	s += std::to_string(ast.data[node].class_declaration.class_id);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

void
class_declaration_predefine(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint class_id = ast.data[node].class_declaration.class_id;
	if (!type_check_state.def_class(class_id))
	{
		err_at_token(ast.tokens[node], "Type Error",
			"Class %d has already been predefined",
			class_id);
	}
}

void
class_declaration_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	size_t byte_size = 0;

	uint field_idx = ast.data[node].class_declaration.fields_ed_idx;
	uint len       = ast.extra_data[field_idx];
	for (uint i = field_idx + 1; i < field_idx + 1 + len; i++)
	{
		uint field_node = ast.extra_data[i];
		ast_type_check(ast, field_node, type_check_state);
		byte_size += ast.types[field_node].byte_size(ast.extra_data);
	}

	ClassDefinition class_def(byte_size);

	for (uint i = field_idx + 1; i < field_idx + 1 + len; i++)
	{
		uint field_node = ast.extra_data[i];
		class_def.add_field(ast.data[field_node].type_identifier_pair.identifier_id, ast.types[field_node]);
	}

	type_check_state.add_class(ast.data[node].class_declaration.class_id, class_def, ast.extra_data);
}

#endif