#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/VariableDeclaration.hpp"

struct FunctionDeclaration final : public ASTNode
{
	std::unique_ptr<TypeIdentifierPair> type_and_id_pair;
	std::vector<std::unique_ptr<TypeIdentifierPair>> params;
	std::unique_ptr<CodeBlock> body;
	FunctionSignature fn_signature;
	uint64_t locals_size = 0;

	FunctionDeclaration(
		std::unique_ptr<TypeIdentifierPair> type_and_id_pair,
		std::vector<std::unique_ptr<TypeIdentifierPair>> &&params,
		std::unique_ptr<CodeBlock> body)
		: ASTNode(type_and_id_pair->accountable_token, FUNCTION_DECLARATION),
		  type_and_id_pair(std::move(type_and_id_pair)),
		  params(std::move(params)),
		  body(std::move(body)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		type_and_id_pair->dfs(callback, depth + 1);

		for (std::unique_ptr<TypeIdentifierPair> &param : params)
		{
			param->dfs(callback, depth + 1);
		}

		body->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "FunctionDeclaration {} @ ";
		s += to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type_and_id_pair->type_check(type_check_state);

		Type return_type           = type_and_id_pair->type;
		fn_signature               = FunctionSignature(
                        type_and_id_pair->get_identifier_name(), return_type);

		// Add parameters

		for (std::unique_ptr<TypeIdentifierPair> &param : params)
		{
			param->type_check(type_check_state);

			std::string param_name = param->get_identifier_name();
			fn_signature.parameters.push_back(
				IdentifierDefinition(param_name, param->type));
		}

		const std::string &fn_name = type_and_id_pair->get_identifier_name();
		if (!type_check_state.add_function(fn_name, fn_signature))
		{
			err_at_token(accountable_token,
				"Duplicate identifier name",
				"Identifier %s is already declared",
				fn_name.c_str());
		}
	}

	void
	post_type_check(TypeCheckState &type_check_state)
		override
	{
		for (std::unique_ptr<TypeIdentifierPair> &param : params)
		{
			std::string param_name = param->get_identifier_name();
			type_check_state.add_parameter(param_name, param->type);
		}

		// Type check the function body

		const std::string &fn_name = type_and_id_pair->get_identifier_name();
		type_check_state.begin_function_scope(fn_name);

		body->type_check(type_check_state);

		locals_size = type_check_state.locals_size;
		type_check_state.end_function_scope();
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		const std::string &fn_name = type_and_id_pair->get_identifier_name();

		assembler.add_label(fn_name);
		if (assembler.debug)
		{
			assembler.label(fn_name);
		}

		// Make space for the locals on the stack

		if (locals_size > 0)
		{
			assembler.allocate_stack(locals_size);
		}

		// Compile the function body

		body->code_gen(assembler);

		assembler.return_();
	}
};

#endif