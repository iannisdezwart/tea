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
			err_at_token(accountable_token,
				"Type Error",
				"Argument count does not equal parameter count"
				"Function %s expects %ld arguments, got %ld",
				fn_signature.id.name.c_str(), fn_signature.parameters.size(),
				arguments.size());
		}

		for (std::unique_ptr<ReadValue> &arg : arguments)
		{
			arg->type_check(type_check_state);
		}
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		uint8_t arg_reg;

		size_t args_size = 0;

		if (arguments.size())
			arg_reg = assembler.get_register();

		for (size_t i = 0; i < arguments.size(); i++)
		{
			const Type &param_type = fn_signature.parameters[i].type;
			const Type &arg_type   = arguments[i]->type;

			std::string param = param_type.to_str();
			std::string arg   = arg_type.to_str();

			if (!arg_type.fits(param_type))
			{
				err_at_token(accountable_token,
					"Type Error",
					"Function call arguments list don't fit "
					"function parameter type template\n"
					"argument[%lu] is of type %s. Expected type %s",
					i, arg.c_str(), param.c_str());
			}

			size_t byte_size = param_type.byte_size();

			// Put value into the argument register

			arguments[i]->get_value(assembler, arg_reg);

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
				err_at_token(accountable_token,
					"Type Error",
					"Function call argument does not fit in a register\n"
					"Behaviour is not implemented yet\n"
					"argument[%lu] is of type %s (width = %lu)",
					i, arg.c_str(), arg_type.byte_size());
			}

			args_size += byte_size;
		}

		if (arguments.size())
		{
			assembler.free_register(arg_reg);
		}

		assembler.push_64(args_size);
		assembler.call(fn_signature.id.name);
		assembler.move_reg_into_reg(R_RET, result_reg);
	}
};

#endif