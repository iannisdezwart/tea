#ifndef TEA_AST_NODE_FUNCTION_DECLARATION_HEADER
#define TEA_AST_NODE_FUNCTION_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "TypeIdentifierPair.hpp"
#include "CodeBlock.hpp"
#include "VariableDeclaration.hpp"

using namespace std;

class FunctionDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		vector<TypeIdentifierPair *> params;
		CodeBlock *body;

		FunctionDeclaration(
			TypeIdentifierPair *type_and_id_pair,
			vector<TypeIdentifierPair *> params,
			CodeBlock *body
		) {
			this->type_and_id_pair = type_and_id_pair;
			this->params = params;
			this->body = body;
			type = FUNCTION_DECLARATION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			if (type_and_id_pair != NULL)
				type_and_id_pair->dfs(callback, depth + 1);

			for (TypeIdentifierPair *param : params) {
				if (param != NULL)
					param->dfs(callback, depth + 1);
			}

			if (body != NULL)
				body->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "FunctionDeclaration {} @ " + to_hex((size_t) this);
			return s;
		}

		Function get_type()
		{
			Type return_type = type_and_id_pair->get_type();
			Function fn_type(return_type);

			// Add parameters

			for (TypeIdentifierPair *param : params) {
				string param_name = param->get_identifier_name();
				Type param_type = param->get_type();
				fn_type.parameter_types.push_back(param_type);
			}

			return fn_type;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			string fn_name = type_and_id_pair->get_identifier_name();
			Function fn_type = get_type();

			if (!compiler_state.add_function(fn_name, fn_type))
				err_at_token(type_and_id_pair->identifier_token,
					"Duplicate identifier name",
					"Identifier %s is already declared",
					fn_name.c_str());

			assembler.add_label(fn_name);

			// Gather locals

			size_t locals_size = 0;

			dfs([&compiler_state, &locals_size](ASTNode *node, size_t depth) {
				if (node->type == VARIABLE_DECLARATION) {
					VariableDeclaration *local = (VariableDeclaration *) node;
					string local_name = local->type_and_id_pair->get_identifier_name();
					Type local_type = local->type_and_id_pair->get_type();

					if (!compiler_state.add_local(local_name, local_type, locals_size))
						err_at_token(local->type_and_id_pair->identifier_token,
							"Duplicate identifier name",
							"Identifier %s is already declared",
							local_name.c_str());

					locals_size += local_type.byte_size();
				}
			}, 0);

			// Make space for the locals on the stack

			assembler.add_64_into_reg(locals_size, R_STACK_P_ID);

			// Compile the function body

			body->compile(assembler, compiler_state);
		}
};

#endif