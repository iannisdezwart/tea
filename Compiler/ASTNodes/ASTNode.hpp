#ifndef TEA_AST_NODE_HEADER
#define TEA_AST_NODE_HEADER

#include "Compiler/tokeniser.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/code-gen/Assembler.hpp"

enum ASTNodeType
{
	TYPE_NAME,
	TYPE_IDENTIFIER_PAIR,
	FUNCTION_DECLARATION,
	VARIABLE_DECLARATION,
	CLASS_DECLARATION,
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
	ASSIGNMENT_EXPRESSION,
	IF_STATEMENT,
	WHILE_STATEMENT,
	FOR_STATEMENT,
	CAST_EXPRESSION,
	OFFSET_EXPRESSION,
	SYS_CALL,
	BREAK_STATEMENT,
	CONTINUE_STATEMENT,
};

const char *
ast_node_type_to_str(ASTNodeType type)
{
	switch (type)
	{
	case TYPE_NAME:
		return "TYPE_NAME";
	case TYPE_IDENTIFIER_PAIR:
		return "TypeIdentifierPair";
	case FUNCTION_DECLARATION:
		return "FunctionDeclaration";
	case VARIABLE_DECLARATION:
		return "VariableDeclaration";
	case CLASS_DECLARATION:
		return "ClassDeclaration";
	case RETURN_STATEMENT:
		return "ReturnStatement";
	case CODE_BLOCK:
		return "CodeBlock";
	case LITERAL_STRING_EXPRESSION:
		return "LiteralStringExpression";
	case LITERAL_CHAR_EXPRESSION:
		return "LiteralCharExpression";
	case LITERAL_NUMBER_EXPRESSION:
		return "LiteralNumberExpression";
	case IDENTIFIER_EXPRESSION:
		return "IdentifierExpression";
	case FUNCTION_CALL:
		return "FunctionCall";
	case BINARY_OPERATION:
		return "Binary Operation";
	case UNARY_OPERATION:
		return "UnaryOperation";
	case MEMBER_EXPRESSION:
		return "MemberExpression";
	case ASSIGNMENT_EXPRESSION:
		return "AssignmentExpression";
	case IF_STATEMENT:
		return "IfStatement";
	case WHILE_STATEMENT:
		return "WhileStatement";
	case FOR_STATEMENT:
		return "ForStatement";
	case CAST_EXPRESSION:
		return "CastExpression";
	case OFFSET_EXPRESSION:
		return "OffsetExpression";
	case SYS_CALL:
		return "SysCall";
	case BREAK_STATEMENT:
		return "BreakStatement";
	case CONTINUE_STATEMENT:
		return "ContinueStatement";
	default:
		return "Undefined";
	}
}

struct ASTNode
{
	Token accountable_token;
	ASTNodeType node_type;
	Type type;

	ASTNode(Token accountable_token, ASTNodeType node_type)
		: accountable_token(std::move(accountable_token)),
		  node_type(node_type) {}

	virtual ~ASTNode() {}

	virtual std::string
	to_str() = 0;

	virtual void
	dfs(std::function<void(ASTNode *, size_t)>, size_t depth = 0) = 0;

	virtual void
	pre_type_check(TypeCheckState &type_check_state)
	{
	}

	virtual void
	type_check(TypeCheckState &type_check_state) = 0;

	virtual void
	post_type_check(TypeCheckState &type_check_state)
	{
	}

	virtual void
	code_gen(Assembler &assembler)
		const = 0;

	void
	print(const char *prefix)
	{
		p_trace(stdout, "%s: %s\n", prefix, to_str().c_str());
	}
};

#endif