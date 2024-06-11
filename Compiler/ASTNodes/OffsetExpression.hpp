#ifndef TEA_AST_NODE_OFFSET_EXPRESSION_HEADER
#define TEA_AST_NODE_OFFSET_EXPRESSION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
offset_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].offset_expression.object_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].offset_expression.offset_node, callback, depth + 1);
	callback(node, depth);
}

std::string
offset_expression_to_str(const AST &ast, uint node)
{
	return std::string("OffsetExpression {} @ ") + std::to_string(node);
}

void
offset_expression_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint object_node = ast.data[node].offset_expression.object_node;
	uint offset_node = ast.data[node].offset_expression.offset_node;

	ast_type_check(ast, object_node, type_check_state);
	ast_type_check(ast, offset_node, type_check_state);

	if (ast.types[offset_node].pointer_depth(ast.extra_data))
	{
		err_at_token(ast.tokens[node],
			"Type Error",
			"Offset provided in OffsetExpression is not an integer\n"
			"Found type %s instead",
			ast.types[offset_node].to_str(ast.extra_data).c_str());
	}

	ast.types[node] = ast.types[object_node].pointed_type(ast.extra_data);
}

void
offset_expression_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint8_t offset_reg = assembler.get_register();

	// Multiply the offset by the byte size.

	uint offset_node = ast.data[node].offset_expression.offset_node;
	ast_get_value(ast, offset_node, assembler, offset_reg);

	uint pointed_type_byte_size = ast.types[node].pointed_type(ast.extra_data).byte_size(ast.extra_data);

	uint8_t temp_reg = assembler.get_register();
	assembler.move_lit(pointed_type_byte_size, temp_reg);
	assembler.mul_int_64(temp_reg, offset_reg);
	assembler.free_register(temp_reg);

	// Add the offset into the pointer.

	uint object_node = ast.data[node].offset_expression.object_node;
	ast_get_value(ast, object_node, assembler, result_reg);
	assembler.add_int_64(offset_reg, result_reg);
	assembler.free_register(offset_reg);

	// Dereference the pointer.

	switch (pointed_type_byte_size)
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
offset_expression_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	// Multiply the offset by the byte size.

	uint8_t offset_reg = assembler.get_register();
	uint8_t temp_reg   = assembler.get_register();

	uint offset_node = ast.data[node].offset_expression.offset_node;
	ast_get_value(ast, offset_node, assembler, offset_reg);
	uint byte_size = ast.types[node].byte_size(ast.extra_data);
	assembler.move_lit(byte_size, temp_reg);
	assembler.mul_int_64(temp_reg, offset_reg);

	// Add the offset into the pointer.

	uint object_node = ast.data[node].offset_expression.object_node;
	ast_get_value(ast, object_node, assembler, temp_reg);
	assembler.add_int_64(offset_reg, temp_reg);
	assembler.free_register(offset_reg);

	// Store the value in the pointed location.

	switch (byte_size)
	{
	case 1:
		assembler.store_ptr_8(value_reg, temp_reg);
		break;

	case 2:
		assembler.store_ptr_16(value_reg, temp_reg);
		break;

	case 4:
		assembler.store_ptr_32(value_reg, temp_reg);
		break;

	case 8:
		assembler.store_ptr_64(value_reg, temp_reg);
		break;

	default:
		assembler.mem_copy(value_reg, temp_reg, byte_size);
		break;
	}

	assembler.free_register(temp_reg);
}

#endif