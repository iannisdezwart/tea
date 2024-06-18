#ifndef TEA_AST_NODES_MEMBER_EXPRESSION_HEADER
#define TEA_AST_NODES_MEMBER_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "VM/cpu.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
member_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].member_expression.object_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].member_expression.member.node, callback, depth + 1); // TODO: reading from union, problematic
	callback(node, depth);
}

std::string
member_expression_direct_to_str(const AST &ast, uint node)
{
	return std::string("MemberExpressionDirect {} @ ") + std::to_string(node);
}

std::string
member_expression_direct_final_to_str(const AST &ast, uint node)
{
	return std::string("MemberExpressionDirectFinal {} @ ") + std::to_string(node);
}

std::string
member_expression_dereferenced_to_str(const AST &ast, uint node)
{
	return std::string("MemberExpressionDereferenced {} @ ") + std::to_string(node);
}

std::string
member_expression_dereferenced_final_to_str(const AST &ast, uint node)
{
	return std::string("MemberExpressionDereferencedFinal {} @ ") + std::to_string(node);
}

template <typename F>
void
member_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state, F &&f)
{
	uint object_node = ast.data[node].member_expression.object_node;
	uint member_node = ast.data[node].member_expression.member.node;

	// assert(ast.tags[member_node] == AstTag::IDENTIFIER_EXPRESSION_STANDALONE); // TODO: REMOVE

	ast.tags[member_node]                                          = AstTag::IDENTIFIER_EXPRESSION_MEMBER;
	ast.data[member_node].identifier_expression_member.object.node = object_node;

	ast_type_check(ast, object_node, type_check_state);
	ast_type_check(ast, member_node, type_check_state);

	uint member_id = ast.data[member_node].identifier_expression_member.identifier_id;

	if (ast.types[object_node].value < BUILTIN_TYPE_END)
	{
		err_at_token(ast.tokens[node], "Type Error",
			"Cannot access property %d from non-class type %s",
			member_id,
			ast_tag_to_str(ast.tags[object_node]));
	}

	size_t offset = 0;

	f(ast, node, type_check_state);

	const ClassDefinition &cl = type_check_state.classes[ast.types[object_node].value];

	for (size_t i = 0; i < cl.fields.size(); i++)
	{
		if (cl.fields[i].id == member_id)
		{
			ast.types[node]                                = cl.fields[i].type;
			ast.data[node].member_expression.member.offset = offset;
			return;
		}

		offset += cl.fields[i].type.byte_size(ast.extra_data);
	}

	err_at_token(ast.tokens[node], "Member error",
		"Class %d has no member %d",
		ast.types[object_node].value, member_id);
}

void
member_expression_direct_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	auto fn = [](AST &ast, uint node, TypeCheckState &type_check_state)
	{
		if (ast.types[ast.data[node].member_expression.object_node].pointer_depth(ast.extra_data) != 0)
		{
			err_at_token(ast.tokens[node], "Invalid MemberExpression",
				"Cannot use x.y syntax on a pointer\n"
				"Use x->y instead");
		}
	};
	member_expression_type_check(ast, node, type_check_state, fn);
}

void
member_expression_dereferenced_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	auto fn = [](AST &ast, uint node, TypeCheckState &type_check_state)
	{
		uint object_node = ast.data[node].member_expression.object_node;
		if (ast.types[object_node].pointer_depth(ast.extra_data) == 0)
		{
			err_at_token(ast.tokens[node], "Invalid MemberExpression",
				"Cannot use x->y syntax on an instance\n"
				"Use x.y instead");
		}
		if (ast.types[object_node].pointer_depth(ast.extra_data) != 1)
		{
			err_at_token(ast.tokens[node], "Invalid MemberExpression",
				"Cannot use x->y syntax on a pointer of depth %u\n",
				ast.types[object_node].pointer_depth(ast.extra_data));
		}
	};
	member_expression_type_check(ast, node, type_check_state, fn);
}

void
member_expression_base_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	// Get address of the class field

	uint object_node = ast.data[node].member_expression.object_node;
	ast_get_value(ast, object_node, assembler, result_reg);

	uint8_t offset_reg = assembler.get_register();
	assembler.move_lit(ast.data[node].member_expression.member.offset, offset_reg);
	assembler.add_int_64(offset_reg, result_reg);
	assembler.free_register(offset_reg);
}

void
member_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	member_expression_base_get_value(ast, node, assembler, result_reg);

	// Dereference

	switch (ast.types[node].byte_size(ast.extra_data))
	{
	case 1:
		assembler.load_ptr_8(result_reg, result_reg);
		break;
	case 2:
		assembler.load_ptr_16(result_reg, result_reg);
		break;
	case 4:
		assembler.load_ptr_32(result_reg, result_reg);
		break;
	case 8:
		assembler.load_ptr_64(result_reg, result_reg);
		break;
	default:
		break;
	}
}

void
member_expression_final_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	member_expression_base_get_value(ast, node, assembler, result_reg);
}

void
member_expression_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	// Get address of the class field

	uint8_t dst_ptr_reg = assembler.get_register();

	uint object_node = ast.data[node].member_expression.object_node;
	ast_get_value(ast, object_node, assembler, dst_ptr_reg);

	uint offset        = ast.data[node].member_expression.member.offset;
	uint8_t offset_reg = assembler.get_register();
	assembler.move_lit(offset, offset_reg);
	assembler.add_int_64(offset_reg, dst_ptr_reg);
	assembler.free_register(offset_reg);

	// Store value

	switch (ast.types[node].byte_size(ast.extra_data))
	{
	case 1:
		assembler.store_ptr_8(value_reg, dst_ptr_reg);
		break;
	case 2:
		assembler.store_ptr_16(value_reg, dst_ptr_reg);
		break;
	case 4:
		assembler.store_ptr_32(value_reg, dst_ptr_reg);
		break;
	case 8:
		assembler.store_ptr_64(value_reg, dst_ptr_reg);
		break;
	default:
		assembler.mem_copy(value_reg, dst_ptr_reg, ast.types[node].byte_size(ast.extra_data));
		break;
	}

	assembler.free_register(dst_ptr_reg);
}

#endif