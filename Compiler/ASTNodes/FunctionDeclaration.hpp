#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "TypeIdentifierPair.hpp"
#include "CodeBlock.hpp"
#include "VariableDeclaration.hpp"

struct FunctionDeclaration : public ASTNode
{
	TypeIdentifierPair *type_and_id_pair;
	std::vector<TypeIdentifierPair *> params;
	CodeBlock *body;

	FunctionDeclaration(
		TypeIdentifierPair *type_and_id_pair,
		std::vector<TypeIdentifierPair *> &params,
		CodeBlock *body)
		: params(params), type_and_id_pair(type_and_id_pair), body(body),
		  ASTNode(type_and_id_pair->identifier_token, FUNCTION_DECLARATION) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
	{
		if (type_and_id_pair != NULL)
		{
			type_and_id_pair->dfs(callback, depth + 1);
		}

		for (TypeIdentifierPair *param : params)
		{
			if (param != NULL)
				param->dfs(callback, depth + 1);
		}

		if (body != NULL)
		{
			body->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
	{
		std::string s = "FunctionDeclaration {} @ ";
		s += to_hex((size_t) this);
		return s;
	}

	Function
	get_fn_type(CompilerState &compiler_state)
	{
		Type return_type = type_and_id_pair->get_type(compiler_state);
		Function fn_type(type_and_id_pair->get_identifier_name(),
			return_type);

		// Add parameters

		for (TypeIdentifierPair *param : params)
		{
			std::string param_name = param->get_identifier_name();
			Type param_type        = param->get_type(compiler_state);
			fn_type.parameters.push_back(
				Identifier(param_name, param_type));
		}

		return fn_type;
	}

	Type
	get_type(CompilerState &compiler_state)
	{
		return Type();
	}

	void
	define(CompilerState &compiler_state)
	{
		const std::string &fn_name = type_and_id_pair->get_identifier_name();
		Function fn_type           = get_fn_type(compiler_state);

		if (!compiler_state.add_function(fn_name, fn_type))
		{
			err_at_token(type_and_id_pair->identifier_token,
				"Duplicate identifier name",
				"Identifier %s is already declared",
				fn_name.c_str());
		}
	}

	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
		if (compiler_state.scope_depth > 0)
		{
			err_at_token(type_and_id_pair->identifier_token,
				"Nested function error",
				"Functions cannot be declared within other functions");
		}

		const std::string &fn_name           = type_and_id_pair->get_identifier_name();
		compiler_state.current_function_name = fn_name;

		assembler.add_label(fn_name);
		if (compiler_state.debug)
		{
			assembler.label(fn_name);
		}

		// Gather parameters

		for (size_t i = 0; i < params.size(); i++)
		{
			std::string param_name = params[i]->get_identifier_name();
			Type param_type        = params[i]->get_type(compiler_state);

			compiler_state.add_parameter(param_name, param_type);
		}

		// Gather locals

		auto cb = [&compiler_state](ASTNode *node, size_t depth)
		{
			if (node->type != VARIABLE_DECLARATION)
			{
				return;
			}

			VariableDeclaration *local = (VariableDeclaration *) node;
			std::string local_name     = local->type_and_id_pair->get_identifier_name();
			Type local_type            = local->type_and_id_pair->get_type(compiler_state);

			if (!compiler_state.add_local(local_name, local_type))
			{
				err_at_token(local->type_and_id_pair->identifier_token,
					"Duplicate identifier name",
					"Identifier %s is already declared",
					local_name.c_str());
			}
		};

		dfs(cb, 0);

		// Make space for the locals on the stack

		if (compiler_state.locals_size)
		{
			assembler.allocate_stack(compiler_state.locals_size);
		}

		// Compile the function body

		compiler_state.scope_depth++;
		body->compile(assembler, compiler_state);
		compiler_state.scope_depth--;

		// Todo: check branching and return statements

		assembler.return_();

		// Clear parameters and globals from the compiler state

		compiler_state.end_function_scope();
	}
};

#endif