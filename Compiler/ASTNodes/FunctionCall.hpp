#ifndef TEA_AST_NODE_FUNCTION_CALL_HEADER
#define TEA_AST_NODE_FUNCTION_CALL_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"

struct FunctionCall : public ReadValue
{
	Token fn_token;
	std::vector<ReadValue *> arguments;

	FunctionCall(Token &fn_token, std::vector<ReadValue *> &arguments)
		: fn_token(fn_token), arguments(arguments),
		  ReadValue(fn_token, FUNCTION_CALL) {}

	std::string &
	get_name()
	{
		return fn_token.value;
	}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		for (size_t i = 0; i < arguments.size(); i++)
		{
			arguments[i]->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "FunctionCall { callee = \"" + get_name() + "\" } @ "
			+ to_hex((size_t) this);
		return s;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		std::string &fn_name = get_name();

		if (!compiler_state.functions.count(fn_name))
		{
			err_at_token(fn_token, "Call to undeclared function",
				"Function %s was called, but not declared",
				fn_name.c_str());
		}

		return compiler_state.functions[fn_name].id.type;
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state,
		uint8_t result_reg)
	{
		uint8_t arg_reg;

		std::string &fn_name = get_name();

		if (!compiler_state.functions.count(fn_name))
		{
			err_at_token(fn_token, "Call to undeclared function",
				"Function %s was called, but not declared",
				fn_name.c_str());
		}

		Function &fn                    = compiler_state.functions[fn_name];
		std::vector<Identifier> &params = fn.parameters;

		// Validate arguments

		if (arguments.size() != params.size())
		{
			err_at_token(fn_token,
				"Type Error",
				"Argument count does not equal parameter count"
				"Function %s expects %ld arguments, got %ld",
				fn_name.c_str(), params.size(), arguments.size());
		}

		size_t args_size = 0;

		if (arguments.size())
			arg_reg = assembler.get_register();

		for (size_t i = 0; i < arguments.size(); i++)
		{
			Type &param_type = params[i].type;
			Type arg_type    = arguments[i]->get_type(compiler_state);

			std::string param = param_type.to_str();
			std::string arg   = arg_type.to_str();

			if (!arg_type.fits(param_type))
			{
				err_at_token(fn_token,
					"Type Error",
					"Function call arguments list don't fit "
					"function parameter type template\n"
					"argument[%lu] is of type %s. Expected type %s",
					i, arg.c_str(), param.c_str());
			}

			size_t byte_size = param_type.byte_size();

			// Put value into the argument register

			arguments[i]->get_value(assembler, compiler_state, arg_reg);

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
				err_at_token(fn_token,
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
		assembler.call(fn_name);
		assembler.move_reg_into_reg(R_RET, result_reg);
	}
};

#endif