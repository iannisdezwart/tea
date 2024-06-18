#ifndef TEA_AST_NODES_UNARY_OPERATION_HEADER
#define TEA_AST_NODES_UNARY_OPERATION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/AssignmentExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "VM/cpu.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
unary_operation_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].unary_operation.expression_node, callback, depth + 1);
	callback(node, depth);
}

std::string
unary_operation_to_str(const AST &ast, uint node, const char *op)
{
	std::string s = "UnaryOperation { op = \"";
	s += op;
	s += "\" } @ ";
	s += std::to_string(node);
	return s;
}

void
unary_operation_type_check_base(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_type_check(ast, expression_node, type_check_state);
	ast.types[node] = ast.types[expression_node];
}

void
unary_operation_type_check_primitive(AST &ast, uint node, TypeCheckState &type_check_state)
{
	unary_operation_type_check_base(ast, node, type_check_state);

	if (ast.types[node].is_class(ast.extra_data))
	{
		err_at_token(ast.tokens[node], "Internal Error",
			"Unsupported type (%s) for %s.\n"
			"Expected a primitive type",
			ast.types[node].to_str(ast.extra_data).c_str(),
			ast_tag_to_str(ast.tags[node]));
	}
}

void
unary_operation_type_check_logical_not(AST &ast, uint node, TypeCheckState &type_check_state)
{
	unary_operation_type_check_base(ast, node, type_check_state);

	if (ast.types[node].is_class(ast.extra_data))
	{
		err_at_token(ast.tokens[node], "Internal Error",
			"Unsupported type (%s) for %s.\n"
			"Expected a primitive type or pointer",
			ast.types[node].to_str(ast.extra_data).c_str(),
			ast_tag_to_str(ast.tags[node]));
	}
}

void
unary_operation_type_check_dereference(AST &ast, uint node, TypeCheckState &type_check_state)
{
	unary_operation_type_check_base(ast, node, type_check_state);

	if (ast.types[node].pointer_depth(ast.extra_data) == 0)
	{
		err_at_token(ast.tokens[node],
			"Cannot dereference a non-pointer",
			"type %s cannot be dereferenced",
			ast.types[node].to_str(ast.extra_data).c_str());
	}

	uint old_array_sizes_len = ast.extra_data[ast.types[node].array_sizes_idx];

	if (old_array_sizes_len == 1)
	{
		// Pointer type becomes non-pointer type.
		ast.types[node].array_sizes_idx = -1;
		return;
	}

	// Remove a layer of pointer depth.
	uint array_sizes_idx     = ast.extra_data.size();

	// Copy the array sizes but skip the last element.
	ast.extra_data.push_back(old_array_sizes_len - 1);
	for (int i = array_sizes_idx + 1; i < array_sizes_idx + old_array_sizes_len; i++)
	{
		ast.extra_data.push_back(ast.extra_data[i]);
	}

	ast.types[node].array_sizes_idx = array_sizes_idx;
}

void
unary_operation_type_check_address_of(AST &ast, uint node, TypeCheckState &type_check_state)
{
	unary_operation_type_check_base(ast, node, type_check_state);

	uint array_sizes_idx = ast.extra_data.size();

	if (ast.types[node].array_sizes_idx == -1)
	{
		// Non-pointer type becomes pointer type.
		ast.extra_data.push_back(1);
		ast.extra_data.push_back(0);
	}
	else
	{
		// Already pointer type will have one more pointer depth.
		uint old_array_sizes_len = ast.extra_data[ast.types[node].array_sizes_idx];

		// Copy the array sizes but prepend a 0.
		ast.extra_data.push_back(old_array_sizes_len + 1);
		ast.extra_data.push_back(0);
		for (int i = array_sizes_idx + 1; i < array_sizes_idx + old_array_sizes_len + 1; i++)
		{
			ast.extra_data.push_back(ast.extra_data[i]);
		}
	}

	ast.types[node].array_sizes_idx = array_sizes_idx;
}

void
increment(AST &ast, Assembler &assembler, const Type &type, uint8_t result_reg)
{
	if (type.pointer_depth(ast.extra_data) > 0)
	{
		uint8_t temp_reg = assembler.get_register();
		assembler.move_lit(type.pointed_byte_size(), temp_reg);
		assembler.add_int_64(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 1)
	{
		assembler.inc_int_8(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 2)
	{
		assembler.inc_int_16(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 4)
	{
		assembler.inc_int_32(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 8)
	{
		assembler.inc_int_64(result_reg);
	}
	else if (type.value == F32)
	{
		uint8_t temp_reg = assembler.get_register();
		float one        = 1.0f;
		assembler.move_lit(*reinterpret_cast<uint32_t *>(&one), temp_reg);
		assembler.add_flt_32(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
	else if (type.value == F64)
	{
		uint8_t temp_reg = assembler.get_register();
		double one       = 1.0;
		assembler.move_lit(*reinterpret_cast<uint64_t *>(&one), temp_reg);
		assembler.add_flt_64(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
}

void
decrement(AST &ast, Assembler &assembler, const Type &type, uint8_t result_reg)
{
	if (type.pointer_depth(ast.extra_data) > 0)
	{
		uint8_t temp_reg = assembler.get_register();
		assembler.move_lit(type.pointed_byte_size(), temp_reg);
		assembler.sub_int_64(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 1)
	{
		assembler.dec_int_8(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 2)
	{
		assembler.dec_int_16(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 4)
	{
		assembler.dec_int_32(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 8)
	{
		assembler.dec_int_64(result_reg);
	}
	else if (type.value == F32)
	{
		uint8_t temp_reg = assembler.get_register();
		float one        = 1.0f;
		assembler.move_lit(*reinterpret_cast<uint32_t *>(&one), temp_reg);
		assembler.sub_flt_32(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
	else if (type.value == F64)
	{
		uint8_t temp_reg = assembler.get_register();
		double one       = 1.0;
		assembler.move_lit(*reinterpret_cast<uint64_t *>(&one), temp_reg);
		assembler.sub_flt_64(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
}

void
unary_operation_postfix_increment_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	const Type &type     = ast.types[expression_node];

	ast_get_value(ast, expression_node, assembler, result_reg);
	increment(ast, assembler, type, result_reg);
	ast_store(ast, expression_node, assembler, result_reg);
	decrement(ast, assembler, type, result_reg);
}

void
unary_operation_postfix_decrement_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	const Type &type     = ast.types[expression_node];

	ast_get_value(ast, expression_node, assembler, result_reg);
	decrement(ast, assembler, type, result_reg);
	ast_store(ast, expression_node, assembler, result_reg);
	increment(ast, assembler, type, result_reg);
}

void
unary_operation_prefix_increment_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	const Type &type     = ast.types[expression_node];

	ast_get_value(ast, expression_node, assembler, result_reg);
	increment(ast, assembler, type, result_reg);
	ast_store(ast, expression_node, assembler, result_reg);
}

void
unary_operation_prefix_decrement_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	const Type &type     = ast.types[expression_node];

	ast_get_value(ast, expression_node, assembler, result_reg);
	decrement(ast, assembler, type, result_reg);
	ast_store(ast, expression_node, assembler, result_reg);
}

void
unary_operation_plus_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);
}

void
unary_operation_minus_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);

	const Type &type = ast.types[expression_node];

	// Two's complement for ints and XOR for floats.

	if (type.is_integer() && type.byte_size(ast.extra_data) == 1)
	{
		assembler.neg_int_8(result_reg);
		assembler.inc_int_8(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 2)
	{
		assembler.neg_int_16(result_reg);
		assembler.inc_int_16(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 4)
	{
		assembler.neg_int_32(result_reg);
		assembler.inc_int_32(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 8)
	{
		assembler.neg_int_64(result_reg);
		assembler.inc_int_64(result_reg);
	}
	else if (type.value == F32)
	{
		uint8_t temp_reg  = assembler.get_register();
		uint32_t sign_bit = 0x80000000;
		assembler.move_lit(sign_bit, temp_reg);
		assembler.xor_int_32(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
	else if (type.value == F64)
	{
		uint8_t temp_reg  = assembler.get_register();
		uint64_t sign_bit = 0x8000000000000000;
		assembler.move_lit(sign_bit, temp_reg);
		assembler.xor_int_64(temp_reg, result_reg);
		assembler.free_register(temp_reg);
	}
}

void
unary_operation_not_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);

	const Type &type = ast.types[expression_node];

	if (type.pointer_depth(ast.extra_data) > 0)
	{
		assembler.neg_int_64(result_reg);
	}
	if (type.is_integer() && type.byte_size(ast.extra_data) == 1)
	{
		assembler.neg_int_8(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 2)
	{
		assembler.neg_int_16(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 4)
	{
		assembler.neg_int_32(result_reg);
	}
	else if (type.is_integer() && type.byte_size(ast.extra_data) == 8)
	{
		assembler.neg_int_64(result_reg);
	}
}

void
unary_operation_dereference_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	// Moves the address of what to dereference into the result reg.

	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);

	// Move the dereferenced value into the result reg.

	const Type &type = ast.types[expression_node];
	switch (type.byte_size(ast.extra_data, 1))
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
unary_operation_address_of_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint expression_node = ast.data[node].unary_operation.expression_node;
	ast_get_value(ast, expression_node, assembler, result_reg);

	// TODO: IMPLEMENT
}

void
unary_operation_prefix_increment_decrement_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	ast_store(ast, ast.data[node].unary_operation.expression_node, assembler, value_reg);
}

void
unary_operation_dereference_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	uint8_t ptr_reg = assembler.get_register();

	size_t deref_dep = 0;
	uint expr_node   = node;

	while (ast.tags[expr_node] == AstTag::UNARY_OPERATION_DEREFERENCE)
	{
		expr_node = ast.data[node].unary_operation.expression_node;
		deref_dep++;
	}

	// Move the address of what to dereference into the pointer register.

	ast_get_value(ast, expr_node, assembler, ptr_reg);

	// Dereference.

	while (--deref_dep > 0)
	{
		assembler.load_ptr_64(ptr_reg, ptr_reg);
	}

	switch (ast.types[expr_node].byte_size(ast.extra_data, deref_dep))
	{
	case 1:
		assembler.store_ptr_8(value_reg, ptr_reg);
		break;

	case 2:
		assembler.store_ptr_16(value_reg, ptr_reg);
		break;

	case 4:
		assembler.store_ptr_32(value_reg, ptr_reg);
		break;

	case 8:
		assembler.store_ptr_64(value_reg, ptr_reg);
		break;

	default:
		// TODO: Test if this works.
		assembler.mem_copy(value_reg, ptr_reg, ast.types[expr_node].byte_size(ast.extra_data, deref_dep));
		break;
	}

	assembler.free_register(ptr_reg);
}

#endif