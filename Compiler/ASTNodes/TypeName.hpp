#ifndef TEA_AST_NODE_TYPE_NAME_HEADER
#define TEA_AST_NODE_TYPE_NAME_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
type_name_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
type_name_to_str(const AST &ast, uint node)
{
	std::string s = "TypeName { type_id = \"";
	s += std::to_string(ast.data[node].type_name.type_id);
	s += "\" } @ ";
	s += std::to_string(node);
	return s;
}

void
type_name_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_id = ast.data[node].type_name.type_id;

	if (type_id >= BUILTIN_TYPE_END)
	{
		const ClassDefinition &class_def = type_check_state.classes[type_id];
		uint byte_size                   = class_def.byte_size;
		ast.types[node]                  = Type(type_id, byte_size);
		return;
	}

	ast.types[node] = Type::type_from_builtin(static_cast<BuiltinType>(type_id));
}

std::string
type_name_indirection_to_str(const AST &ast, uint node)
{
	uint type_id       = ast.data[node].type_name_indirection.type_id;
	uint arr_sizes_idx = ast.data[node].type_name_indirection.arr_sizes_ed_idx;
	uint arr_sizes_len = ast.extra_data[arr_sizes_idx];

	std::string s = "TypeNameIndirection { type_id = ";
	s += std::to_string(type_id);
	s += ", arr_sizes = [";

	for (uint i = arr_sizes_idx + 1; i < arr_sizes_idx + 1 + arr_sizes_len; i++)
	{
		s += std::to_string(ast.extra_data[i]);
		if (i != arr_sizes_idx + arr_sizes_len)
			s += ", ";
	}

	s += "] } @ ";
	s += std::to_string(node);
	return s;
}

void
type_name_indirection_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint type_id = ast.data[node].type_name_indirection.type_id;
	uint arr_sizes_idx = ast.data[node].type_name_indirection.arr_sizes_ed_idx;

	if (type_id >= BUILTIN_TYPE_END)
	{
		const ClassDefinition &class_def = type_check_state.classes[type_id];
		uint byte_size                   = class_def.byte_size;
		ast.types[node]                  = Type(type_id, byte_size, arr_sizes_idx);
		return;
	}

	ast.types[node] = Type::type_from_builtin(static_cast<BuiltinType>(type_id), arr_sizes_idx);
}

#endif