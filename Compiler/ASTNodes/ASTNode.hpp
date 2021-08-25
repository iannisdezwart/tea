#ifndef TEA_AST_NODE_HEADER
#define TEA_AST_NODE_HEADER

#include <bits/stdc++.h>

#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"

using namespace std;

enum ASTNodeType {
	TYPE_NAME,
	TYPE_IDENTIFIER_PAIR,
	FUNCTION_DECLARATION,
	VARIABLE_DECLARATION,
	CLASS_DECLARATION,
	INIT_LIST,
	RETURN_STATEMENT,
	CODE_BLOCK,
	LITERAL_STRING_EXPRESSION,
	LITERAL_CHAR_EXPRESSION,
	LITERAL_NUMBER_EXPRESSION,
	IDENTIFIER_EXPRESSION,
	FUNCTION_CALL,
	BINARY_OPERATION,
	UNARY_OPERATION,
	MEMBER_EXPRESSION,
	METHOD_CALL,
	ASSIGNMENT_EXPRESSION,
	IF_STATEMENT,
	WHILE_STATEMENT,
	CAST_EXPRESSION,
	OFFSET_EXPRESSION
};

const char *ast_node_type_to_str(enum ASTNodeType type)
{
	switch (type) {
		case TYPE_NAME: return "TYPE_NAME";
		case TYPE_IDENTIFIER_PAIR: return "TypeIdentifierPair";
		case FUNCTION_DECLARATION: return "FunctionDeclaration";
		case VARIABLE_DECLARATION: return "VariableDeclaration";
		case CLASS_DECLARATION: return "ClassDeclaration";
		case RETURN_STATEMENT: return "ReturnStatement";
		case CODE_BLOCK: return "CodeBlock";
		case LITERAL_STRING_EXPRESSION: return "LiteralStringExpression";
		case LITERAL_CHAR_EXPRESSION: return "LiteralCharExpression";
		case LITERAL_NUMBER_EXPRESSION: return "LiteralNumberExpression";
		case IDENTIFIER_EXPRESSION: return "IdentifierExpression";
		case FUNCTION_CALL: return "FunctionCall";
		case BINARY_OPERATION: return "Binary Operation";
		case UNARY_OPERATION: return "UnaryOperation";
		case ASSIGNMENT_EXPRESSION: return "AssignmentExpression";
		case IF_STATEMENT: return "IfStatement";
		case WHILE_STATEMENT: return "WhileStatement";
		case CAST_EXPRESSION: return "CastExpression";
		default: return "Undefined";
	}
}

class ASTNode {
	public:
		ASTNodeType type;
		Token accountable_token;

		ASTNode(const Token& accountable_token, ASTNodeType type)
			: accountable_token(accountable_token), type(type) {}

		virtual string to_str() = 0;
		virtual void dfs(function<void(ASTNode *, size_t)>, size_t depth = 0) = 0;
		virtual void compile(Assembler& assembler, CompilerState& compiler_state) = 0;
		virtual Type get_type(CompilerState& compiler_state) = 0;

		void print(const char *prefix)
		{
			printf("%s: %s\n", prefix, to_str().c_str());
		}
};

#endif