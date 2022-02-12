#ifndef TEA_AST_NODE_CODE_BLOCK_HEADER
#define TEA_AST_NODE_CODE_BLOCK_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"

class CodeBlock : public ASTNode {
	public:
		std::vector<ASTNode *> statements;
		Token start_token;

		CodeBlock(Token start_token) : start_token(start_token),
			ASTNode(start_token, CODE_BLOCK) {}

		~CodeBlock() {}

		void add_statement(ASTNode *statement)
		{
			statements.push_back(statement);
		}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			for (ASTNode *statement : statements) {
				if (statement != NULL) statement->dfs(callback, depth + 1);
			}

			callback(this, depth);
		}

		std::string to_str()
		{
			std::string s = "CodeBlock {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return Type();
		}

		void compile(Assembler& assembler, CompilerState& compiler_state)
		{
			for (ASTNode *statement : statements) {
				statement->compile(assembler, compiler_state);
			}
		}
};

#endif