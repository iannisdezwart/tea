#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
identifier_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	callback(node, depth);
}

std::string
identifier_expression_standalone_to_str(const AST &ast, uint node)
{
	std::string s = "IdentifierExpression { identifier_id = ";
	s += std::to_string(ast.data[node].identifier_expression_standalone.identifier_id);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

std::string
identifier_expression_member_to_str(const AST &ast, uint node)
{
	std::string s = "IdentifierExpression { identifier_id = ";
	s += std::to_string(ast.data[node].identifier_expression_member.identifier_id);
	s += ", object_node = ";
	s += std::to_string(ast.data[node].identifier_expression_member.object.node); // TODO: reading from union, problematic
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

// clang-format off
enum struct IdentifierExpressionIdKind { STACK, GLOBAL };
// clang-format on

void
identifier_expression_standalone_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint identifier_id = ast.data[node].identifier_expression_standalone.identifier_id;
	ast.types[node]    = type_check_state.get_type_of_identifier(identifier_id);

	if (ast.types[node].value == UNDEFINED)
	{
		err_at_token(ast.tokens[node],
			"Identifier has unknown kind",
			"Identifier: %d. this might be a bug in the compiler",
			identifier_id);
	}

	IdentifierKind id_kind = type_check_state.get_identifier_kind(identifier_id);

	if (id_kind == IdentifierKind::UNDEFINED)
	{
		err_at_token(ast.tokens[node],
			"Reference to undeclared variable",
			"Identifier %d was referenced, but not declared",
			identifier_id);
	}

	int offset;
	uint id_kind_bit;

	switch (id_kind)
	{
	case IdentifierKind::LOCAL:
	{
		const VariableDefinition &var = type_check_state.get_local(identifier_id);
		offset                        = var.offset;
		id_kind_bit                   = (uint) IdentifierExpressionIdKind::STACK;
		break;
	}

	case IdentifierKind::PARAMETER:
	{
		const VariableDefinition &var = type_check_state.parameters[identifier_id];
		offset                        = -type_check_state.parameters_size + var.offset
			- 8 - STACK_FRAME_SIZE;
		id_kind_bit = (uint) IdentifierExpressionIdKind::STACK;
		break;
	}

	case IdentifierKind::GLOBAL:
	{
		const VariableDefinition &var = type_check_state.globals[identifier_id];
		offset                        = var.offset;
		id_kind_bit                   = (uint) IdentifierExpressionIdKind::GLOBAL;
		break;
	}

	default:
		err_at_token(ast.tokens[node],
			"Identifier has unknown kind",
			"Identifier: %d. this might be a bug in the compiler",
			identifier_id);
	}

	ast.data[node].identifier_expression_standalone.id_kind = id_kind_bit;
	ast.data[node].identifier_expression_standalone.offset  = offset;
}

void
identifier_expression_member_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint object_node                 = ast.data[node].identifier_expression_member.object.node;
	uint class_id                    = ast.types[object_node].value;
	const ClassDefinition &class_def = type_check_state.classes.at(class_id);

	uint identifier_id = ast.data[node].identifier_expression_member.identifier_id;

	if (!class_def.has_field(identifier_id))
	{
		err_at_token(ast.tokens[node],
			"Class field not found",
			"Field %d not found in class %d",
			identifier_id, class_id);
	}

	ast.types[node]                                           = class_def.get_field_type(identifier_id);
	ast.data[node].identifier_expression_member.object.offset = class_def.get_field_offset(identifier_id, ast.extra_data);
}

void
identifier_expression_standalone_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	int offset       = ast.data[node].identifier_expression_standalone.offset;
	uint id_kind_bit = ast.data[node].identifier_expression_standalone.id_kind;

	if (id_kind_bit == (uint) IdentifierExpressionIdKind::STACK)
	{
		switch (ast.types[node].byte_size(ast.extra_data))
		{
		case 1:
			assembler.move_lit(offset, result_reg);
			assembler.add_int_64(R_FRAME_PTR, result_reg);
			assembler.load_ptr_8(result_reg, result_reg);
			break;

		case 2:
			assembler.move_lit(offset, result_reg);
			assembler.add_int_64(R_FRAME_PTR, result_reg);
			assembler.load_ptr_16(result_reg, result_reg);
			break;

		case 4:
			assembler.move_lit(offset, result_reg);
			assembler.add_int_64(R_FRAME_PTR, result_reg);
			assembler.load_ptr_32(result_reg, result_reg);
			break;

		case 8:
			assembler.move_lit(offset, result_reg);
			assembler.add_int_64(R_FRAME_PTR, result_reg);
			assembler.load_ptr_64(result_reg, result_reg);
			break;

		default:
			assembler.move_lit(offset, result_reg);
			assembler.add_int_64(R_FRAME_PTR, result_reg);
			break;
		}

		return;
	}

	switch (ast.types[node].byte_size(ast.extra_data))
	{
	case 1:
		assembler.move_lit(offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
		assembler.load_ptr_8(result_reg, result_reg);
		break;

	case 2:
		assembler.move_lit(offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
		assembler.load_ptr_16(result_reg, result_reg);
		break;

	case 4:
		assembler.move_lit(offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
		assembler.load_ptr_32(result_reg, result_reg);
		break;

	case 8:
		assembler.move_lit(offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
		assembler.load_ptr_64(result_reg, result_reg);
		break;

	default:
		assembler.move_lit(offset, result_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, result_reg);
		break;
	}
}

void
identifier_expression_standalone_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	int offset       = ast.data[node].identifier_expression_standalone.offset;
	uint id_kind_bit = ast.data[node].identifier_expression_standalone.id_kind;

	if (id_kind_bit == (uint) IdentifierExpressionIdKind::STACK)
	{
		switch (ast.types[node].byte_size(ast.extra_data))
		{
		case 1:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(offset, dst_ptr_reg);
			assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
			assembler.store_ptr_8(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 2:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(offset, dst_ptr_reg);
			assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
			assembler.store_ptr_16(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 4:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(offset, dst_ptr_reg);
			assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
			assembler.store_ptr_32(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		case 8:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(offset, dst_ptr_reg);
			assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
			assembler.store_ptr_64(value_reg, dst_ptr_reg);
			assembler.free_register(dst_ptr_reg);
			break;
		}

		default:
		{
			uint8_t dst_ptr_reg = assembler.get_register();
			assembler.move_lit(offset, dst_ptr_reg);
			assembler.add_int_64(R_FRAME_PTR, dst_ptr_reg);
			assembler.mem_copy(value_reg, dst_ptr_reg, ast.types[node].byte_size(ast.extra_data));
			assembler.free_register(dst_ptr_reg);
			break;
		}
		}

		return;
	}

	// Global variable.

	switch (ast.types[node].byte_size(ast.extra_data))
	{
	case 1:
	{
		uint8_t dst_ptr_reg = assembler.get_register();
		assembler.move_lit(offset, dst_ptr_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
		assembler.store_ptr_8(value_reg, dst_ptr_reg);
		assembler.free_register(dst_ptr_reg);
		break;
	}

	case 2:
	{
		uint8_t dst_ptr_reg = assembler.get_register();
		assembler.move_lit(offset, dst_ptr_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
		assembler.store_ptr_16(value_reg, dst_ptr_reg);
		assembler.free_register(dst_ptr_reg);
		break;
	}

	case 4:
	{
		uint8_t dst_ptr_reg = assembler.get_register();
		assembler.move_lit(offset, dst_ptr_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
		assembler.store_ptr_32(value_reg, dst_ptr_reg);
		assembler.free_register(dst_ptr_reg);
		break;
	}

	case 8:
	{
		uint8_t dst_ptr_reg = assembler.get_register();
		assembler.move_lit(offset, dst_ptr_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
		assembler.store_ptr_64(value_reg, dst_ptr_reg);
		assembler.free_register(dst_ptr_reg);
		break;
	}

	default:
	{
		uint8_t dst_ptr_reg = assembler.get_register();
		assembler.move_lit(offset, dst_ptr_reg);
		assembler.add_int_64(R_STACK_TOP_PTR, dst_ptr_reg);
		assembler.mem_copy(value_reg, dst_ptr_reg, ast.types[node].byte_size(ast.extra_data));
		assembler.free_register(dst_ptr_reg);
		break;
	}
	}
}

#endif