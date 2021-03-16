#ifndef TEA_AST_NODE_FUNCTION_CALL_HEADER
#define TEA_AST_NODE_FUNCTION_CALL_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"

using namespace std;

class FunctionCall : public ASTNode {
	public:
		Token fn_token;
		vector<ASTNode *> arguments;

		FunctionCall(Token& fn_token, vector<ASTNode *>& arguments)
			: fn_token(fn_token), arguments(arguments)
		{
			type = FUNCTION_CALL;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		string& get_name()
		{
			return fn_token.value;
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			for (size_t i = 0; i < arguments.size(); i++) {
				arguments[i]->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		string to_str()
		{
			string s = "FunctionCall { callee = \"" + get_name() + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			string& fn_name = get_name();

			if (!compiler_state.functions.count(fn_name))
				err_at_token(fn_token, "Call to undeclared function",
					"Identifier %s was called, but not declared",
					fn_name.c_str());

			return compiler_state.functions[fn_name].return_type;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			string& fn_name = get_name();

			if (!compiler_state.functions.count(fn_name))
				err_at_token(fn_token, "Call to undeclared function",
					"Identifier %s was called, but not declared",
					fn_name.c_str());

			Function& fn = compiler_state.functions[fn_name];
			vector<Type>& param_types = fn.parameter_types;

			if (arguments.size() != param_types.size())
				err_at_token(fn_token,
					"Argument count does not equal parameter count",
					"Function %s expects %ld arguments, got %ld",
					fn_name.c_str(), param_types.size(), arguments.size());

			size_t args_size = 0;

			for (size_t i = 0; i < arguments.size(); i++) {
				Type& param_type = param_types[i];
				Type arg_type = arguments[i]->get_type(compiler_state);

				string param = param_type.to_str();
				string arg = arg_type.to_str();

				if (param_type != arg_type)
					err_at_token(fn_token,
						"Function call arguments list does not match"
						"function parameter type template",
						"argument[%lu] is of type %s. Expected type %s",
						i, arg.c_str(), param.c_str());

				size_t byte_size = param_type.byte_size();

				// Stores result into R_ACCUMULATOR_0_ID

				arguments[i]->compile(assembler, compiler_state);

				switch (byte_size) {
					case 1:
						assembler.push_reg_8(R_ACCUMULATOR_0_ID);
						break;

					case 2:
						assembler.push_reg_16(R_ACCUMULATOR_0_ID);
						break;

					case 4:
						assembler.push_reg_32(R_ACCUMULATOR_0_ID);
						break;

					case 8:
						assembler.push_reg_64(R_ACCUMULATOR_0_ID);
						break;

					default:
						err_at_token(fn_token,
							"Function call argument does not fit in a register",
							"Behaviour is not implemented yet\n"
							"argument[%lu] is of type %s (width = %lu)",
							i, arg.c_str(), arg_type.byte_size());
				}

				args_size += byte_size;
			}

			assembler.push_64(args_size);
			assembler.call(fn_name);
		}
};

#endif