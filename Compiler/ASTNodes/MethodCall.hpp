#ifndef TEA_AST_NODE_METHOD_CALL_HEADER
#define TEA_AST_NODE_METHOD_CALL_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "IdentifierExpression.hpp"
#include "FunctionCall.hpp"
#include "UnaryOperation.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"

struct MethodCall : public ReadValue
{
	IdentifierExpression *object;
	FunctionCall *method;
	Token op_token;
	enum Operator op;

	MethodCall(IdentifierExpression *object, FunctionCall *method,
		const Token &op_token)
		: object(object), method(method), op_token(op_token),
		  ReadValue(op_token, METHOD_CALL) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		object->dfs(callback, depth + 1);
		method->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "MethodCall { callee = \"";
		s += object->identifier_token.value;
		s += "::" + method->get_name() + "\" } @ ";
		s += to_hex((size_t) this);
		return s;
	}

	std::string
	get_object_class_name(CompilerState &compiler_state)
	{
		const std::string &obj_name = object->identifier_token.value;
		Type type                   = compiler_state.get_type_of_identifier(obj_name);

		if (type != Type::USER_DEFINED_CLASS)
		{
			err_at_token(object->accountable_token, "Type Error",
				"Cannot access methods from %s of type %s\n"
				"Methods are only accessible from classes",
				obj_name.c_str(), type.to_str().c_str());
		}

		return type.class_name;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		const std::string &class_name  = get_object_class_name(compiler_state);
		const Class &cl                = compiler_state.classes[class_name];
		const std::string &method_name = method->get_name();

		if (!cl.has_method(method_name))
		{
			err_at_token(method->accountable_token, "Call to undeclared method",
				"Method %s::%s was called, but not declared",
				class_name.c_str(), method_name.c_str());
		}

		return cl.get_method(method_name).id.type;
	}

	void
	get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t res_reg)
	{
		uint8_t arg_reg;

		std::string fn_name = get_object_class_name(compiler_state) + "::" + method->get_name();

		if (!compiler_state.functions.count(fn_name))
		{
			err_at_token(method->accountable_token, "Call to undeclared method",
				"Method %s was called, but not declared",
				fn_name.c_str());
		}

		Function &fn                    = compiler_state.functions[fn_name];
		std::vector<Identifier> &params = fn.parameters;

		// Add the this pointer to the argument list

		Token address_of_token;
		address_of_token.type  = OPERATOR;
		address_of_token.value = "&";

		IdentifierExpression *obj_id = new IdentifierExpression(*object);
		UnaryOperation *this_pointer = new UnaryOperation(obj_id, address_of_token, true);
		method->arguments.insert(method->arguments.begin(), this_pointer);

		// Validate arguments

		if (method->arguments.size() != params.size())
		{
			err_at_token(method->fn_token,
				"Type Error",
				"Argument count does not equal parameter count"
				"Method %s expects %ld arguments, got %ld",
				fn_name.c_str(), params.size(), method->arguments.size());
		}

		size_t args_size = 0;

		if (method->arguments.size())
		{
			arg_reg = assembler.get_register();
		}

		for (size_t i = 0; i < method->arguments.size(); i++)
		{
			Type &param_type = params[i].type;
			Type arg_type    = method->arguments[i]->get_type(compiler_state);

			std::string param = param_type.to_str();
			std::string arg   = arg_type.to_str();

			if (param_type != arg_type)
				err_at_token(method->fn_token,
					"Type Error",
					"Method call arguments list does not match "
					"method parameter type template\n"
					"argument[%lu] is of type %s. Expected type %s",
					i, arg.c_str(), param.c_str());

			size_t byte_size = param_type.byte_size();

			// Stores result into the argument register

			method->arguments[i]->get_value(assembler, compiler_state, arg_reg);

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
				err_at_token(method->fn_token,
					"Type Error",
					"Function call argument does not fit in a register\n"
					"Behaviour is not implemented yet\n"
					"argument[%lu] is of type %s (width = %lu)",
					i, arg.c_str(), arg_type.byte_size());
			}

			args_size += byte_size;
		}

		if (method->arguments.size())
		{
			assembler.free_register(arg_reg);
		}

		assembler.push_64(args_size);
		assembler.call(fn_name);
	}
};

#endif