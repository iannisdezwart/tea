#ifndef TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER
#define TEA_AST_NODES_ASSIGNMENT_EXPRESSION_HEADER

#include <functional>

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "VM/cpu.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
assignment_expression_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].assignment_expression.store_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].assignment_expression.value_node, callback, depth + 1);
	callback(node, depth);
}

std::string
assignment_expression_to_str(const AST &ast, uint node, const char *op)
{
	std::string s = "AssignmentExpression { op = \"";
	s += op;
	s += "\" } @ ";
	s += std::to_string(node);
	return s;
}

void
assignment_expression_type_check_base(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint store_node = ast.data[node].assignment_expression.store_node;
	uint value_node = ast.data[node].assignment_expression.value_node;

	ast_type_check(ast, store_node, type_check_state);
	ast_type_check(ast, value_node, type_check_state);

	Type::Fits type_fits = ast.types[store_node].fits(ast.types[value_node], ast.extra_data);

	if (type_fits == Type::Fits::NO)
		warn("At %s, Right hand side value of AssignmentExpression does not "
		     "fit into left hand side value\n"
		     "lhs_type = %s, rhs_type = %s",
			ast.tokens[store_node].to_str().c_str(),
			ast.types[store_node].to_str(ast.extra_data).c_str(),
			ast.types[value_node].to_str(ast.extra_data).c_str());

	ast.types[node] = ast.types[store_node];
}

// Sum, difference, product, quotient, remainder.
void
assignment_expression_type_check_arith(AST &ast, uint node, TypeCheckState &type_check_state)
{
	assignment_expression_type_check_base(ast, node, type_check_state);

	if (!ast.types[node].is_integer() && !ast.types[node].is_float())
		err_at_token(ast.tokens[node],
			"Type Error",
			"Assignment expression %s is not defined for type %s",
			ast_tag_to_str(ast.tags[node]), ast.types[node].to_str(ast.extra_data).c_str());
}

// Left shift, right shift, bitwise and, bitwise or, bitwise xor.
void
assignment_expression_type_check_bitwise(AST &ast, uint node, TypeCheckState &type_check_state)
{
	assignment_expression_type_check_base(ast, node, type_check_state);

	if (!ast.types[node].is_integer())
		err_at_token(ast.tokens[node],
			"Type Error",
			"%s is not defined for type %s",
			ast_tag_to_str(ast.tags[node]), ast.types[node].to_str(ast.extra_data).c_str());
}

void
assignment_expression_base_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	// Moves result into its register

	uint value_node = ast.data[node].assignment_expression.value_node;
	ast_get_value(ast, value_node, assembler, result_reg);

	// Implicit type casting

	uint store_node      = ast.data[node].assignment_expression.store_node;
	Type::Fits type_fits = ast.types[store_node].fits(ast.types[value_node], ast.extra_data);

	if (type_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
		assembler.cast_flt_32_to_int(result_reg);
	else if (type_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
		assembler.cast_flt_64_to_int(result_reg);
	else if (type_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
		assembler.cast_int_to_flt_32(result_reg);
	else if (type_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
		assembler.cast_int_to_flt_64(result_reg);
}

void
assignment_expression_normal_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node = ast.data[node].assignment_expression.store_node;
	ast_store(ast, store_node, assembler, result_reg);
}

void
assignment_expression_sum_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.add_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.add_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.add_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.add_int_64(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F32)
	{
		assembler.add_flt_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F64)
	{
		assembler.add_flt_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_difference_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.sub_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.sub_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.sub_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.sub_int_64(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F32)
	{
		assembler.sub_flt_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F64)
	{
		assembler.sub_flt_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_product_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.mul_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.mul_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.mul_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.mul_int_64(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F32)
	{
		assembler.mul_flt_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F64)
	{
		assembler.mul_flt_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_quotient_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.div_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.div_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.div_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.div_int_64(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F32)
	{
		assembler.div_flt_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].value == F64)
	{
		assembler.div_flt_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_remainder_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.mod_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.mod_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.mod_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.mod_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_left_shift_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.shl_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.shl_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.shl_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.shl_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_right_shift_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.shr_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.shr_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.shr_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.shr_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_bitwise_and_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.and_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.and_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.and_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.and_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_bitwise_or_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.or_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.or_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.or_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.or_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

void
assignment_expression_bitwise_xor_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	assignment_expression_base_get_value(ast, node, assembler, result_reg);

	uint store_node   = ast.data[node].assignment_expression.store_node;
	uint prev_val_reg = assembler.get_register();
	ast_get_value(ast, store_node, assembler, prev_val_reg);

	if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 1)
	{
		assembler.xor_int_8(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 2)
	{
		assembler.xor_int_16(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 4)
	{
		assembler.xor_int_32(result_reg, prev_val_reg);
	}
	else if (ast.types[node].is_integer() && ast.types[node].byte_size(ast.extra_data) == 8)
	{
		assembler.xor_int_64(result_reg, prev_val_reg);
	}

	ast_store(ast, store_node, assembler, prev_val_reg);
	assembler.free_register(prev_val_reg);
}

#endif