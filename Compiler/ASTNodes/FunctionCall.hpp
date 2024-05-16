#ifndef TEA_AST_NODE_FUNCTION_CALL_HEADER
#define TEA_AST_NODE_FUNCTION_CALL_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"

struct FunctionCall final : public ReadValue
{
	std::vector<std::unique_ptr<ReadValue>> arguments;
	FunctionSignature fn_signature;

	FunctionCall(Token fn_token, std::vector<std::unique_ptr<ReadValue>> &&arguments)
		: ReadValue(std::move(fn_token), FUNCTION_CALL),
		  arguments(std::move(arguments)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (size_t i = 0; i < arguments.size(); i++)
		{
			arguments[i]->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "FunctionCall { callee = \"" + accountable_token.value + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{

		if (!type_check_state.functions.count(accountable_token.value))
		{
			err_at_token(accountable_token, "Call to undeclared function",
				"Function %s was called, but not declared",
				accountable_token.value.c_str());
		}

		fn_signature = type_check_state.functions[accountable_token.value];
		type         = fn_signature.id.type;

		// Validate arguments

		if (arguments.size() != fn_signature.parameters.size())
		{
			warn("At %s, Argument count does not equal parameter count"
			     "Function %s expects %ld arguments, got %ld",
				accountable_token.to_str().c_str(),
				fn_signature.id.name.c_str(), fn_signature.parameters.size(),
				arguments.size());
		}

		for (size_t i = 0; i < std::min(fn_signature.parameters.size(), arguments.size()); i++)
		{
			const std::unique_ptr<ReadValue> &arg = arguments[i];
			const Type &param_type                = fn_signature.parameters[i].type;
			arg->type_check(type_check_state);

			if (arg->type.fits(param_type) == Type::Fits::NO)
			{
				warn("At %s, Function call arguments list don't fit "
				     "function parameter type template\n"
				     "argument %lu is of type %s. Expected type %s",
					accountable_token.to_str().c_str(),
					i + 1, arg->type.to_str().c_str(), param_type.to_str().c_str());
			}
		}
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		uint8_t arg_reg;

		size_t args_size = 0;

		if (std::min(fn_signature.parameters.size(), arguments.size()))
			arg_reg = assembler.get_register();

		for (size_t i = 0; i < std::min(fn_signature.parameters.size(), arguments.size()); i++)
		{
			const Type &param_type = fn_signature.parameters[i].type;
			const Type &arg_type   = arguments[i]->type;

			std::string param = param_type.to_str();
			std::string arg   = arg_type.to_str();

			size_t byte_size = param_type.byte_size();

			// Put value into the argument register

			arguments[i]->get_value(assembler, arg_reg);

			// Implicit type casting

			Type::Fits type_fits = param_type.fits(arg_type);
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

		if (std::min(fn_signature.parameters.size(), arguments.size()))
		{
			assembler.free_register(arg_reg);
		}

		uint8_t args_size_reg = assembler.get_register();
		assembler.move_lit(args_size, args_size_reg);
		assembler.push_reg_64(args_size_reg);
		assembler.free_register(args_size_reg);
		assembler.call(fn_signature.id.name);
		assembler.move(R_RET, result_reg);
	}
};

#endif