#ifndef TEA_AST_NODE_FUNCTION_CALL_HEADER
#define TEA_AST_NODE_FUNCTION_CALL_HEADER

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/util.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/AST.hpp"

void
function_call_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	uint arg_idx = ast.data[node].function_call.ed_idx + 1;
	uint len     = ast.extra_data[arg_idx];
	for (uint i = arg_idx + 1; i < arg_idx + 1 + len; i++)
	{
		uint argument_node = ast.extra_data[i];
		ast_dfs(ast, argument_node, callback, depth + 1);
	}

	callback(node, depth);
}

std::string
function_call_to_str(const AST &ast, uint node)
{
	std::string s = "FunctionCall { callee_id = ";
	s += std::to_string(ast.data[node].function_call.callee_id);
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

void
function_call_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint callee_id = ast.data[node].function_call.callee_id;

	auto it = type_check_state.functions.find(callee_id);
	if (it == type_check_state.functions.end())
	{
		err_at_token(ast.tokens[node], "Call to undeclared function",
			"Function %d was called, but not declared",
			callee_id);
	}

	FunctionSignature fn_signature = it->second;
	ast.types[node]                = fn_signature.id.type;

	// Validate arguments

	uint arg_idx = ast.data[node].function_call.ed_idx + 1;
	uint arg_len = ast.extra_data[arg_idx];

	if (arg_len != fn_signature.parameters.size())
	{
		warn("At %s, Argument count does not equal parameter count\n"
		     "Function %d expects %ld arguments, got %u",
			ast.tokens[node].to_str().c_str(),
			fn_signature.id.id,
			fn_signature.parameters.size(),
			arg_len);
	}

	for (size_t i = 0; i < std::min(static_cast<uint>(fn_signature.parameters.size()), arg_len); i++)
	{
		uint arg_node          = ast.extra_data[arg_idx + 1 + i];
		const Type &param_type = fn_signature.parameters[i].type;
		ast_type_check(ast, arg_node, type_check_state);

		if (ast.types[arg_node].fits(param_type, ast.extra_data) == Type::Fits::NO)
		{
			warn("At %s, Function call arguments list don't fit "
			     "function parameter type template\n"
			     "argument %lu is of type %s. Expected type %s",
				ast.tokens[node].to_str().c_str(),
				i + 1,
				ast.types[arg_node].to_str(ast.extra_data).c_str(),
				param_type.to_str(ast.extra_data).c_str());
		}
	}

	uint fn_signatures_idx = ast.function_signatures.size();
	ast.function_signatures.push_back(fn_signature);
	ast.extra_data[ast.data[node].function_call.ed_idx] = fn_signatures_idx;
}

void
function_call_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	uint ed_idx  = ast.data[node].function_call.ed_idx;
	uint arg_idx = ed_idx + 1;
	uint arg_len = ast.extra_data[arg_idx];

	uint arg_reg;

	size_t args_size = 0;

	uint fn_signature_idx                 = ast.extra_data[ed_idx];
	FunctionSignature &function_signature = ast.function_signatures[fn_signature_idx];

	if (std::min(static_cast<uint>(function_signature.parameters.size()), arg_len))
		arg_reg = assembler.get_register();

	for (size_t i = 0; i < std::min(static_cast<uint>(function_signature.parameters.size()), arg_len); i++)
	{
		uint arg_node          = ast.extra_data[arg_idx + 1 + i];
		const Type &param_type = function_signature.parameters[i].type;
		const Type &arg_type   = ast.types[arg_node];

		size_t byte_size = param_type.byte_size(ast.extra_data);

		// Put value into the argument register

		ast_get_value(ast, arg_node, assembler, arg_reg);

		// Implicit type casting

		Type::Fits type_fits = param_type.fits(arg_type, ast.extra_data);
		if (type_fits == Type::Fits::FLT_32_TO_INT_CAST_NEEDED)
			assembler.cast_flt_32_to_int(result_reg);
		else if (type_fits == Type::Fits::FLT_64_TO_INT_CAST_NEEDED)
			assembler.cast_flt_64_to_int(result_reg);
		else if (type_fits == Type::Fits::INT_TO_FLT_32_CAST_NEEDED)
			assembler.cast_int_to_flt_32(result_reg);
		else if (type_fits == Type::Fits::INT_TO_FLT_64_CAST_NEEDED)
			assembler.cast_int_to_flt_64(result_reg);

		switch (byte_size)
		{
		case 1:
			assembler.push_reg_8(arg_reg);
			break;

		case 2:
			assembler.push_reg_16(arg_reg);
			break;

		case 4:
			assembler.push_reg_32(arg_reg);
			break;

		case 8:
			assembler.push_reg_64(arg_reg);
			break;

		default:
		{
			assembler.mem_copy(arg_reg, R_STACK_PTR, byte_size);
			uint8_t byte_size_reg = assembler.get_register();
			assembler.move_lit(byte_size, byte_size_reg);
			assembler.add_int_64(byte_size_reg, R_STACK_PTR);
			assembler.free_register(byte_size_reg);
			break;
		}
		}

		args_size += byte_size;
	}

	if (std::min(static_cast<uint>(function_signature.parameters.size()), arg_len))
	{
		assembler.free_register(arg_reg);
	}

	uint8_t args_size_reg = assembler.get_register();
	assembler.move_lit(args_size, args_size_reg);
	assembler.push_reg_64(args_size_reg);
	assembler.free_register(args_size_reg);
	assembler.call(function_signature.id.id);
	assembler.move(R_RET, result_reg);
}

#endif