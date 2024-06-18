#ifndef TEA_AST_NODE_BINARY_OPERATION_HEADER
#define TEA_AST_NODE_BINARY_OPERATION_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
binary_operation_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	ast_dfs(ast, ast.data[node].binary_operation.lhs_expr_node, callback, depth + 1);
	ast_dfs(ast, ast.data[node].binary_operation.rhs_expr_node, callback, depth + 1);
	callback(node, depth);
}

std::string
binary_operation_to_str(const AST &ast, uint node, const char *op)
{
	std::string s = "BinaryOperation { op = \"";
	s += op;
	s += "\" } @ ";
	s += std::to_string(node);
	return s;
}

void
binary_operation_type_check_base(AST &ast, uint node, TypeCheckState &type_check_state)
{
	ast_type_check(ast, ast.data[node].binary_operation.lhs_expr_node, type_check_state);
	ast_type_check(ast, ast.data[node].binary_operation.rhs_expr_node, type_check_state);
}

void
binary_operation_type_check_additive(AST &ast, uint node, TypeCheckState &type_check_state)
{
	binary_operation_type_check_base(ast, node, type_check_state);

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;

	// sizeX + sizeY -> max(sizeX, sizeY)

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0)
	{
		if (ast.types[lhs_node].is_float() && !ast.types[rhs_node].is_float())
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		if (ast.types[rhs_node].is_float() && !ast.types[lhs_node].is_float())
		{
			ast.types[node] = ast.types[rhs_node];
			return;
		}

		if (ast.types[lhs_node].byte_size(ast.extra_data) > ast.types[rhs_node].byte_size(ast.extra_data))
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		ast.types[node] = ast.types[rhs_node];
		return;
	}

	if (!ast.types[lhs_node].is_integer() || !ast.types[rhs_node].is_integer())
	{
		warn("%s is not implemented for types x = %s and y = %s\n"
		     "At %u:%u\n",
			ast_tag_to_str(ast.tags[node]),
			ast.types[lhs_node].to_str(ast.extra_data).c_str(),
			ast.types[rhs_node].to_str(ast.extra_data).c_str(),
			ast.tokens[node].line,
			ast.tokens[node].col);
	}

	// pointer + int -> pointer

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) > 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0)
	{
		ast.types[node] = ast.types[lhs_node];
		return;
	}

	// int + pointer -> pointer

	if (ast.types[rhs_node].pointer_depth(ast.extra_data) > 0
		&& ast.types[lhs_node].pointer_depth(ast.extra_data) == 0)
	{
		ast.types[node] = ast.types[rhs_node];
		return;
	}

	ast.types[node] = ast.types[lhs_node];
}

void
binary_operation_type_check_multiplicative(AST &ast, uint node, TypeCheckState &type_check_state)
{
	binary_operation_type_check_base(ast, node, type_check_state);

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;

	// sizeX * sizeY -> max(sizeX, sizeY)

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0)
	{
		if (ast.types[lhs_node].is_float() && !ast.types[rhs_node].is_float())
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		if (ast.types[rhs_node].is_float() && !ast.types[lhs_node].is_float())
		{
			ast.types[node] = ast.types[rhs_node];
			return;
		}

		if (ast.types[lhs_node].byte_size(ast.extra_data) > ast.types[rhs_node].byte_size(ast.extra_data))
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		ast.types[node] = ast.types[rhs_node];
		return;
	}

	ast.types[node] = ast.types[lhs_node];
}

void
binary_operation_type_check_remainder(AST &ast, uint node, TypeCheckState &type_check_state)
{
	binary_operation_type_check_base(ast, node, type_check_state);

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;

	if (!ast.types[lhs_node].is_integer() || !ast.types[rhs_node].is_integer())
	{
		warn("%s is not implemented for types x = %s and y = %s\n"
		     "At %u:%u\n",
			ast_tag_to_str(ast.tags[node]),
			ast.types[lhs_node].to_str(ast.extra_data).c_str(),
			ast.types[rhs_node].to_str(ast.extra_data).c_str(),
			ast.tokens[node].line,
			ast.tokens[node].col);
	}

	// intX % intY -> intX

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0)
	{
		ast.types[node] = ast.types[lhs_node];
		return;
	}

	ast.types[node] = ast.types[rhs_node];
}

void
binary_operation_type_check_bitwise(AST &ast, uint node, TypeCheckState &type_check_state)
{
	binary_operation_type_check_base(ast, node, type_check_state);

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;

	if (!ast.types[lhs_node].is_integer() || !ast.types[rhs_node].is_integer())
	{
		warn("%s is not implemented for types x = %s and y = %s\n"
		     "At %u:%u\n",
			ast_tag_to_str(ast.tags[node]),
			ast.types[lhs_node].to_str(ast.extra_data).c_str(),
			ast.types[rhs_node].to_str(ast.extra_data).c_str(),
			ast.tokens[node].line,
			ast.tokens[node].col);
	}

	// intX & intX

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[lhs_node].byte_size(ast.extra_data) == ast.types[rhs_node].byte_size(ast.extra_data))
	{
		ast.types[node] = ast.types[lhs_node];
		return;
	}

	ast.types[node] = ast.types[rhs_node];
}

void
binary_operation_type_check_comparison(AST &ast, uint node, TypeCheckState &type_check_state)
{
	binary_operation_type_check_base(ast, node, type_check_state);

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;

	if (ast.types[lhs_node].value != ast.types[rhs_node].value
		&& (ast.types[lhs_node].pointer_depth(ast.extra_data)
			!= ast.types[rhs_node].pointer_depth(ast.extra_data)))
	{
		warn("comparing different types x = %s and y = %s\n"
		     "At %u:%u\n",
			ast.types[lhs_node].to_str(ast.extra_data).c_str(),
			ast.types[rhs_node].to_str(ast.extra_data).c_str(),
			ast.tokens[node].line,
			ast.tokens[node].col);
	}

	ast.types[node] = Type(ast.types[lhs_node].value, 1); // Result of comparison is boolean

	if (ast.types[lhs_node].pointer_depth(ast.extra_data) == 0
		&& ast.types[rhs_node].pointer_depth(ast.extra_data) == 0)
	{
		if (ast.types[lhs_node].is_float() && !ast.types[rhs_node].is_float())
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		if (ast.types[rhs_node].is_float() && !ast.types[lhs_node].is_float())
		{
			ast.types[node] = ast.types[rhs_node];
			return;
		}

		if (ast.types[lhs_node].byte_size(ast.extra_data) > ast.types[rhs_node].byte_size(ast.extra_data))
		{
			ast.types[node] = ast.types[lhs_node];
			return;
		}

		ast.types[node] = ast.types[rhs_node];
		return;
	}

	ast.types[node] = ast.types[lhs_node];
}

template <typename F>
void
binary_operation_base_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg, F &&f)
{
	uint8_t rhs_reg;

	// Get the left hand side value.

	uint lhs_node = ast.data[node].binary_operation.lhs_expr_node;
	ast_get_value(ast, lhs_node, assembler, result_reg);

	// Get the right hand side value.

	rhs_reg       = assembler.get_register();
	uint rhs_node = ast.data[node].binary_operation.rhs_expr_node;
	ast_get_value(ast, rhs_node, assembler, rhs_reg);

	// Implicit type casting

	Type::Fits left_fits  = ast.types[lhs_node].fits(ast.types[node], ast.extra_data);
	Type::Fits right_fits = ast.types[rhs_node].fits(ast.types[node], ast.extra_data);

	if (left_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
		assembler.cast_flt_32_to_int(result_reg);
	else if (left_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
		assembler.cast_flt_64_to_int(result_reg);
	else if (left_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
		assembler.cast_int_to_flt_32(result_reg);
	else if (left_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
		assembler.cast_int_to_flt_64(result_reg);

	if (right_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
		assembler.cast_flt_32_to_int(rhs_reg);
	else if (right_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
		assembler.cast_flt_64_to_int(rhs_reg);
	else if (right_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
		assembler.cast_int_to_flt_32(rhs_reg);
	else if (right_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
		assembler.cast_int_to_flt_64(rhs_reg);

	f(ast, node, assembler, result_reg, rhs_reg);

	assembler.free_register(rhs_reg);
}

void
binary_operation_get_value_addition(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.add_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.add_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.add_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.add_int_64(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F32)
		{
			assembler.add_flt_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F64)
		{
			assembler.add_flt_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_subtraction(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.sub_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.sub_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.sub_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.sub_int_64(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F32)
		{
			assembler.sub_flt_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F64)
		{
			assembler.sub_flt_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_multiplication(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.mul_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.mul_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.mul_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.mul_int_64(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F32)
		{
			assembler.mul_flt_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F64)
		{
			assembler.mul_flt_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_division(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.div_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.div_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.div_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.div_int_64(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F32)
		{
			assembler.div_flt_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F64)
		{
			assembler.div_flt_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_remainder(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.mod_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.mod_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.mod_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.mod_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_bitwise_and(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.and_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.and_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.and_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.and_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_bitwise_or(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.or_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.or_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.or_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.or_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_bitwise_xor(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.xor_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.xor_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.xor_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.xor_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_left_shift(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.shl_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.shl_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.shl_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.shl_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

void
binary_operation_get_value_right_shift(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == U8 || ast.types[node].value == I8)
		{
			assembler.shr_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16 || ast.types[node].value == I16)
		{
			assembler.shr_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32 || ast.types[node].value == I32)
		{
			assembler.shr_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64 || ast.types[node].value == I64)
		{
			assembler.shr_int_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
}

template <typename G>
void
binary_operation_get_value_compare_base(AST &ast, uint node, Assembler &assembler, uint8_t result_reg, G &&g)
{
	auto f = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg, uint8_t rhs_reg)
	{
		if (ast.types[node].value == I8)
		{
			assembler.cmp_int_8(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U8)
		{
			assembler.cmp_int_8_u(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == I16)
		{
			assembler.cmp_int_16(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U16)
		{
			assembler.cmp_int_16_u(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == I32)
		{
			assembler.cmp_int_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U32)
		{
			assembler.cmp_int_32_u(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == I64)
		{
			assembler.cmp_int_64(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == U64)
		{
			assembler.cmp_int_64_u(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F32)
		{
			assembler.cmp_flt_32(rhs_reg, result_reg);
		}
		else if (ast.types[node].value == F64)
		{
			assembler.cmp_flt_64(rhs_reg, result_reg);
		}
	};
	binary_operation_base_get_value(ast, node, assembler, result_reg, f);
	g(ast, node, assembler, result_reg);
}

void
binary_operation_get_value_less(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_lt(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

void
binary_operation_get_value_less_or_equal(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_leq(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

void
binary_operation_get_value_greater(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_gt(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

void
binary_operation_get_value_greater_or_equal(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_geq(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

void
binary_operation_get_value_equal(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_eq(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

void
binary_operation_get_value_not_equal(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	auto g = [](AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
	{
		assembler.set_if_neq(result_reg);
	};
	binary_operation_get_value_compare_base(ast, node, assembler, result_reg, g);
}

#endif