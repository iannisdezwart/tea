#ifndef TEA_AST_FUNCTIONS_HEADER
#define TEA_AST_FUNCTIONS_HEADER

#include "ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/AST.hpp"
#include "Compiler/ASTNodes/AssignmentExpression.hpp"
#include "Compiler/ASTNodes/BinaryOperation.hpp"
#include "Compiler/ASTNodes/BreakStatement.hpp"
#include "Compiler/ASTNodes/CastExpression.hpp"
#include "Compiler/ASTNodes/ClassDeclaration.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/ContinueStatement.hpp"
#include "Compiler/ASTNodes/ForStatement.hpp"
#include "Compiler/ASTNodes/FunctionCall.hpp"
#include "Compiler/ASTNodes/FunctionDeclaration.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/IfStatement.hpp"
#include "Compiler/ASTNodes/LiteralCharExpression.hpp"
#include "Compiler/ASTNodes/LiteralNumberExpression.hpp"
#include "Compiler/ASTNodes/LiteralStringExpression.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"
#include "Compiler/ASTNodes/OffsetExpression.hpp"
#include "Compiler/ASTNodes/ReturnStatement.hpp"
#include "Compiler/ASTNodes/SysCall.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/ASTNodes/UnaryOperation.hpp"
#include "Compiler/ASTNodes/VariableDeclaration.hpp"
#include "Compiler/ASTNodes/WhileStatement.hpp"

void
ast_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	switch (ast.tags[node])
	{
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
		assignment_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::BINARY_OPERATION_ADDITION:
	case AstTag::BINARY_OPERATION_SUBTRACTION:
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
	case AstTag::BINARY_OPERATION_DIVISION:
	case AstTag::BINARY_OPERATION_REMAINDER:
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
	case AstTag::BINARY_OPERATION_BITWISE_AND:
	case AstTag::BINARY_OPERATION_BITWISE_OR:
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
	case AstTag::BINARY_OPERATION_LESS:
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
	case AstTag::BINARY_OPERATION_GREATER:
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
	case AstTag::BINARY_OPERATION_EQUAL:
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
		binary_operation_dfs(ast, node, callback, depth);
		break;

	case AstTag::BREAK_STATEMENT:
		break_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::CAST_EXPRESSION:
		cast_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::CLASS_DECLARATION:
		class_declaration_dfs(ast, node, callback, depth);
		break;

	case AstTag::CODE_BLOCK:
		code_block_dfs(ast, node, callback, depth);
		break;

	case AstTag::CONTINUE_STATEMENT:
		continue_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::FOR_STATEMENT:
		for_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::FUNCTION_CALL:
		function_call_dfs(ast, node, callback, depth);
		break;

	case AstTag::FUNCTION_DECLARATION:
		function_declaration_dfs(ast, node, callback, depth);

	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
	case AstTag::IDENTIFIER_EXPRESSION_MEMBER:
		identifier_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::IF_STATEMENT:
		if_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::IF_ELSE_STATEMENT:
		if_else_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::LITERAL_CHAR_EXPRESSION:
		literal_char_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::LITERAL_INTEGER_EXPRESSION:
	case AstTag::LITERAL_FLOAT_EXPRESSION:
		literal_number_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::LITERAL_STRING_EXPRESSION:
		literal_string_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::MEMBER_EXPRESSION_DIRECT:
	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		member_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::OFFSET_EXPRESSION:
		offset_expression_dfs(ast, node, callback, depth);
		break;

	case AstTag::RETURN_VOID_STATEMENT:
		return_void_statement_dfs(ast, node, callback, depth);
		break;
	case AstTag::RETURN_EXPRESSION_STATEMENT:
		return_expression_statement_dfs(ast, node, callback, depth);
		break;

	case AstTag::SYS_CALL:
		sys_call_dfs(ast, node, callback, depth);
		break;

	case AstTag::TYPE_IDENTIFIER_PAIR:
		type_identifier_pair_dfs(ast, node, callback, depth);
		break;

	case AstTag::TYPE_NAME:
	case AstTag::TYPE_NAME_INDIRECTION:
		type_name_dfs(ast, node, callback, depth);
		break;

	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_PLUS:
	case AstTag::UNARY_OPERATION_MINUS:
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
	case AstTag::UNARY_OPERATION_DEREFERENCE:
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
		unary_operation_dfs(ast, node, callback, depth);
		break;

	case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
		variable_declaration_uninitialised_dfs(ast, node, callback, depth);
		break;

	case AstTag::VARIABLE_DECLARATION_INITIALISED:
		variable_declaration_initialised_dfs(ast, node, callback, depth);
		break;

	case AstTag::WHILE_STATEMENT:
		while_statement_dfs(ast, node, callback, depth);
		break;

	default:
		err("ast_dfs - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
	}
}

std::string
ast_to_str(const AST &ast, uint node)
{
	switch (ast.tags[node])
	{
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
		return assignment_expression_to_str(ast, node, "=");
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
		return assignment_expression_to_str(ast, node, "+=");
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
		return assignment_expression_to_str(ast, node, "-=");
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
		return assignment_expression_to_str(ast, node, "*=");
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
		return assignment_expression_to_str(ast, node, "/=");
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
		return assignment_expression_to_str(ast, node, "%=");
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
		return assignment_expression_to_str(ast, node, "<<=");
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
		return assignment_expression_to_str(ast, node, ">>=");
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
		return assignment_expression_to_str(ast, node, "&=");
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
		return assignment_expression_to_str(ast, node, "|=");
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
		return assignment_expression_to_str(ast, node, "^=");

	case AstTag::BINARY_OPERATION_ADDITION:
		return binary_operation_to_str(ast, node, "+");
	case AstTag::BINARY_OPERATION_SUBTRACTION:
		return binary_operation_to_str(ast, node, "-");
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
		return binary_operation_to_str(ast, node, "*");
	case AstTag::BINARY_OPERATION_DIVISION:
		return binary_operation_to_str(ast, node, "/");
	case AstTag::BINARY_OPERATION_REMAINDER:
		return binary_operation_to_str(ast, node, "%");
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
		return binary_operation_to_str(ast, node, "<<");
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
		return binary_operation_to_str(ast, node, ">>");
	case AstTag::BINARY_OPERATION_BITWISE_AND:
		return binary_operation_to_str(ast, node, "&");
	case AstTag::BINARY_OPERATION_BITWISE_OR:
		return binary_operation_to_str(ast, node, "|");
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
		return binary_operation_to_str(ast, node, "^");
	case AstTag::BINARY_OPERATION_LESS:
		return binary_operation_to_str(ast, node, "<");
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
		return binary_operation_to_str(ast, node, "<=");
	case AstTag::BINARY_OPERATION_GREATER:
		return binary_operation_to_str(ast, node, ">");
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
		return binary_operation_to_str(ast, node, ">=");
	case AstTag::BINARY_OPERATION_EQUAL:
		return binary_operation_to_str(ast, node, "==");
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
		return binary_operation_to_str(ast, node, "!=");

	case AstTag::BREAK_STATEMENT:
		return break_statement_to_str(ast, node);

	case AstTag::CAST_EXPRESSION:
		return cast_expression_to_str(ast, node);

	case AstTag::CLASS_DECLARATION:
		return class_declaration_to_str(ast, node);

	case AstTag::CODE_BLOCK:
		return code_block_to_str(ast, node);

	case AstTag::CONTINUE_STATEMENT:
		return continue_statement_to_str(ast, node);

	case AstTag::FOR_STATEMENT:
		return for_statement_to_str(ast, node);

	case AstTag::FUNCTION_CALL:
		return function_call_to_str(ast, node);

	case AstTag::FUNCTION_DECLARATION:
		return function_declaration_to_str(ast, node);

	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
		return identifier_expression_standalone_to_str(ast, node);
	case AstTag::IDENTIFIER_EXPRESSION_MEMBER:
		return identifier_expression_member_to_str(ast, node);

	case AstTag::IF_STATEMENT:
		return if_statement_to_str(ast, node);

	case AstTag::IF_ELSE_STATEMENT:
		return if_else_statement_to_str(ast, node);

	case AstTag::LITERAL_CHAR_EXPRESSION:
		return literal_char_expression_to_str(ast, node);

	case AstTag::LITERAL_INTEGER_EXPRESSION:
		return literal_integer_expression_to_str(ast, node);

	case AstTag::LITERAL_FLOAT_EXPRESSION:
		return literal_float_expression_to_str(ast, node);

	case AstTag::LITERAL_STRING_EXPRESSION:
		return literal_string_expression_to_str(ast, node);

	case AstTag::MEMBER_EXPRESSION_DIRECT:
		return member_expression_direct_to_str(ast, node);

	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
		return member_expression_direct_final_to_str(ast, node);

	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
		return member_expression_dereferenced_to_str(ast, node);

	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		return member_expression_dereferenced_final_to_str(ast, node);

	case AstTag::OFFSET_EXPRESSION:
		return offset_expression_to_str(ast, node);

	case AstTag::RETURN_VOID_STATEMENT:
		return return_void_statement_to_str(ast, node);
	case AstTag::RETURN_EXPRESSION_STATEMENT:
		return return_expression_statement_to_str(ast, node);

	case AstTag::SYS_CALL:
		return sys_call_to_str(ast, node);

	case AstTag::TYPE_IDENTIFIER_PAIR:
		return type_identifier_pair_to_str(ast, node);

	case AstTag::TYPE_NAME:
		return type_name_to_str(ast, node);

	case AstTag::TYPE_NAME_INDIRECTION:
		return type_name_indirection_to_str(ast, node);

	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
		return unary_operation_to_str(ast, node, "++");
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
		return unary_operation_to_str(ast, node, "--");
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
		return unary_operation_to_str(ast, node, "++");
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
		return unary_operation_to_str(ast, node, "--");
	case AstTag::UNARY_OPERATION_PLUS:
		return unary_operation_to_str(ast, node, "+");
	case AstTag::UNARY_OPERATION_MINUS:
		return unary_operation_to_str(ast, node, "-");
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
		return unary_operation_to_str(ast, node, "~");
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
		return unary_operation_to_str(ast, node, "!");
	case AstTag::UNARY_OPERATION_DEREFERENCE:
		return unary_operation_to_str(ast, node, "*");
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
		return unary_operation_to_str(ast, node, "&");

	case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
		return variable_declaration_uninitialised_to_str(ast, node);
	case AstTag::VARIABLE_DECLARATION_INITIALISED:
		return variable_declaration_initialised_to_str(ast, node);

	case AstTag::WHILE_STATEMENT:
		return while_statement_to_str(ast, node);

	default:
		err("ast_to_str - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
		return "";
	}
}

void
ast_print(const AST &ast, uint node, const char *prefix)
{
	p_trace(stdout, "%s: %s\n", prefix, ast_to_str(ast, node).c_str());
}

void
ast_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	switch (ast.tags[node])
	{
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
		assignment_expression_type_check_base(ast, node, type_check_state);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
		assignment_expression_type_check_arith(ast, node, type_check_state);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
		assignment_expression_type_check_bitwise(ast, node, type_check_state);
		break;

	case AstTag::BINARY_OPERATION_ADDITION:
	case AstTag::BINARY_OPERATION_SUBTRACTION:
		binary_operation_type_check_additive(ast, node, type_check_state);
		break;
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
	case AstTag::BINARY_OPERATION_DIVISION:
		binary_operation_type_check_multiplicative(ast, node, type_check_state);
		break;
	case AstTag::BINARY_OPERATION_REMAINDER:
		binary_operation_type_check_remainder(ast, node, type_check_state);
		break;
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
	case AstTag::BINARY_OPERATION_BITWISE_AND:
	case AstTag::BINARY_OPERATION_BITWISE_OR:
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
		binary_operation_type_check_bitwise(ast, node, type_check_state);
		break;

	case AstTag::BINARY_OPERATION_LESS:
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
	case AstTag::BINARY_OPERATION_GREATER:
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
	case AstTag::BINARY_OPERATION_EQUAL:
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
		binary_operation_type_check_comparison(ast, node, type_check_state);
		break;

	case AstTag::BREAK_STATEMENT:
		break;

	case AstTag::CAST_EXPRESSION:
		cast_expression_type_check(ast, node, type_check_state);
		break;

	case AstTag::CLASS_DECLARATION:
		class_declaration_type_check(ast, node, type_check_state);
		break;

	case AstTag::CODE_BLOCK:
		code_block_type_check(ast, node, type_check_state);
		break;

	case AstTag::CONTINUE_STATEMENT:
		break;

	case AstTag::FOR_STATEMENT:
		for_statement_type_check(ast, node, type_check_state);
		break;

	case AstTag::FUNCTION_CALL:
		function_call_type_check(ast, node, type_check_state);
		break;

	case AstTag::FUNCTION_DECLARATION:
		function_declaration_type_check(ast, node, type_check_state);
		break;

	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
		identifier_expression_standalone_type_check(ast, node, type_check_state);
		break;
	case AstTag::IDENTIFIER_EXPRESSION_MEMBER:
		identifier_expression_member_type_check(ast, node, type_check_state);
		break;

	case AstTag::IF_STATEMENT:
		if_statement_type_check(ast, node, type_check_state);
		break;
	case AstTag::IF_ELSE_STATEMENT:
		if_else_statement_type_check(ast, node, type_check_state);
		break;

	case AstTag::LITERAL_CHAR_EXPRESSION:
		literal_char_expression_type_check(ast, node, type_check_state);
		break;
	case AstTag::LITERAL_INTEGER_EXPRESSION:
		literal_integer_expression_type_check(ast, node, type_check_state);
		break;
	case AstTag::LITERAL_FLOAT_EXPRESSION:
		literal_float_expression_type_check(ast, node, type_check_state);
		break;
	case AstTag::LITERAL_STRING_EXPRESSION:
		literal_string_expression_type_check(ast, node, type_check_state);
		break;

	case AstTag::MEMBER_EXPRESSION_DIRECT:
	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
		member_expression_direct_type_check(ast, node, type_check_state);
		break;
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		member_expression_dereferenced_type_check(ast, node, type_check_state);
		break;

	case AstTag::OFFSET_EXPRESSION:
		offset_expression_type_check(ast, node, type_check_state);
		break;

	case AstTag::RETURN_VOID_STATEMENT:
		return_void_statement_type_check(ast, node, type_check_state);
		break;

	case AstTag::RETURN_EXPRESSION_STATEMENT:
		return_expression_statement_type_check(ast, node, type_check_state);
		break;

	case AstTag::SYS_CALL:
		sys_call_type_check(ast, node, type_check_state);
		break;

	case AstTag::TYPE_IDENTIFIER_PAIR:
		type_identifier_pair_type_check(ast, node, type_check_state);
		break;

	case AstTag::TYPE_NAME:
		type_name_type_check(ast, node, type_check_state);
		break;

	case AstTag::TYPE_NAME_INDIRECTION:
		type_name_indirection_type_check(ast, node, type_check_state);
		break;

	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_PLUS:
	case AstTag::UNARY_OPERATION_MINUS:
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
		unary_operation_type_check_primitive(ast, node, type_check_state);
		break;
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
		unary_operation_type_check_logical_not(ast, node, type_check_state);
		break;
	case AstTag::UNARY_OPERATION_DEREFERENCE:
		unary_operation_type_check_dereference(ast, node, type_check_state);
		break;
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
		unary_operation_type_check_address_of(ast, node, type_check_state);
		break;

	case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
		variable_declaration_uninitialised_type_check(ast, node, type_check_state);
		break;
	case AstTag::VARIABLE_DECLARATION_INITIALISED:
		variable_declaration_initialised_type_check(ast, node, type_check_state);
		break;

	case AstTag::WHILE_STATEMENT:
		while_statement_type_check(ast, node, type_check_state);
		break;

	default:
		err("ast_type_check - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
	}
}

void
ast_code_gen(AST &ast, uint node, Assembler &assembler)
{
	switch (ast.tags[node])
	{
	// Read-values
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
	case AstTag::BINARY_OPERATION_ADDITION:
	case AstTag::BINARY_OPERATION_SUBTRACTION:
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
	case AstTag::BINARY_OPERATION_DIVISION:
	case AstTag::BINARY_OPERATION_REMAINDER:
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
	case AstTag::BINARY_OPERATION_BITWISE_AND:
	case AstTag::BINARY_OPERATION_BITWISE_OR:
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
	case AstTag::BINARY_OPERATION_LESS:
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
	case AstTag::BINARY_OPERATION_GREATER:
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
	case AstTag::BINARY_OPERATION_EQUAL:
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
	case AstTag::CAST_EXPRESSION:
	case AstTag::FUNCTION_CALL:
	case AstTag::LITERAL_CHAR_EXPRESSION:
	case AstTag::LITERAL_INTEGER_EXPRESSION:
	case AstTag::LITERAL_FLOAT_EXPRESSION:
	case AstTag::LITERAL_STRING_EXPRESSION:
	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
	case AstTag::IDENTIFIER_EXPRESSION_MEMBER:
	case AstTag::MEMBER_EXPRESSION_DIRECT:
	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
	case AstTag::OFFSET_EXPRESSION:
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
	case AstTag::UNARY_OPERATION_PLUS:
	case AstTag::UNARY_OPERATION_MINUS:
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
	case AstTag::UNARY_OPERATION_DEREFERENCE:
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
	{
		uint8_t result_reg = assembler.get_register();
		ast_get_value(ast, node, assembler, result_reg);
		assembler.free_register(result_reg);
		break;
	}

	case AstTag::BREAK_STATEMENT:
		break_statement_code_gen(assembler);
		break;

	case AstTag::CLASS_DECLARATION:
		break;

	case AstTag::CODE_BLOCK:
		code_block_code_gen(ast, node, assembler);
		break;

	case AstTag::CONTINUE_STATEMENT:
		continue_statement_code_gen(assembler);
		break;

	case AstTag::FOR_STATEMENT:
		for_statement_code_gen(ast, node, assembler);
		break;

	case AstTag::FUNCTION_DECLARATION:
		function_declaration_code_gen(ast, node, assembler);
		break;

	case AstTag::IF_STATEMENT:
		if_statement_code_gen(ast, node, assembler);
		break;

	case AstTag::IF_ELSE_STATEMENT:
		if_else_statement_code_gen(ast, node, assembler);
		break;

	case AstTag::RETURN_VOID_STATEMENT:
		return_void_statement_code_gen(assembler);
		break;

	case AstTag::RETURN_EXPRESSION_STATEMENT:
		return_expression_statement_code_gen(ast, node, assembler);
		break;

	case AstTag::SYS_CALL:
		sys_call_code_gen(ast, node, assembler);
		break;

	case AstTag::TYPE_IDENTIFIER_PAIR:
	case AstTag::TYPE_NAME:
	case AstTag::TYPE_NAME_INDIRECTION:
		break;

	case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
		break;

	case AstTag::VARIABLE_DECLARATION_INITIALISED:
		variable_declaration_initialised_code_gen(ast, node, assembler);
		break;

	case AstTag::WHILE_STATEMENT:
		while_statement_code_gen(ast, node, assembler);
		break;

	default:
		err("ast_code_gen - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
	}
}

void
ast_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg)
{
	switch (ast.tags[node])
	{
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
		assignment_expression_normal_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
		assignment_expression_sum_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
		assignment_expression_difference_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
		assignment_expression_product_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
		assignment_expression_quotient_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
		assignment_expression_remainder_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
		assignment_expression_left_shift_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
		assignment_expression_right_shift_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
		assignment_expression_bitwise_and_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
		assignment_expression_bitwise_or_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
		assignment_expression_bitwise_xor_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::BINARY_OPERATION_ADDITION:
		binary_operation_get_value_addition(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_SUBTRACTION:
		binary_operation_get_value_subtraction(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
		binary_operation_get_value_multiplication(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_DIVISION:
		binary_operation_get_value_division(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_REMAINDER:
		binary_operation_get_value_remainder(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
		binary_operation_get_value_left_shift(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
		binary_operation_get_value_right_shift(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_BITWISE_AND:
		binary_operation_get_value_bitwise_and(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_BITWISE_OR:
		binary_operation_get_value_bitwise_or(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
		binary_operation_get_value_bitwise_xor(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_LESS:
		binary_operation_get_value_less(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
		binary_operation_get_value_less_or_equal(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_GREATER:
		binary_operation_get_value_greater(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
		binary_operation_get_value_greater_or_equal(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_EQUAL:
		binary_operation_get_value_equal(ast, node, assembler, result_reg);
		break;
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
		binary_operation_get_value_not_equal(ast, node, assembler, result_reg);
		break;

	case AstTag::CAST_EXPRESSION:
		cast_expression_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::FUNCTION_CALL:
		function_call_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
		identifier_expression_standalone_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::LITERAL_CHAR_EXPRESSION:
		literal_char_expression_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::LITERAL_INTEGER_EXPRESSION:
		literal_integer_expression_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::LITERAL_FLOAT_EXPRESSION:
		literal_float_expression_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::LITERAL_STRING_EXPRESSION:
		literal_string_expression_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::MEMBER_EXPRESSION_DIRECT:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
		member_expression_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		member_expression_final_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::OFFSET_EXPRESSION:
		offset_expression_get_value(ast, node, assembler, result_reg);
		break;

	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
		unary_operation_postfix_increment_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
		unary_operation_postfix_decrement_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
		unary_operation_prefix_increment_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
		unary_operation_prefix_decrement_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_PLUS:
		unary_operation_plus_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_MINUS:
		unary_operation_minus_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
		unary_operation_not_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_DEREFERENCE:
		unary_operation_dereference_get_value(ast, node, assembler, result_reg);
		break;
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
		unary_operation_address_of_get_value(ast, node, assembler, result_reg);
		break;
	default:
		err("ast_get_value - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
	}
}

void
ast_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg)
{
	switch (ast.tags[node])
	{
	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
		identifier_expression_standalone_store(ast, node, assembler, value_reg);
		break;

	case AstTag::MEMBER_EXPRESSION_DIRECT:
	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		member_expression_store(ast, node, assembler, value_reg);
		break;

	case AstTag::OFFSET_EXPRESSION:
		offset_expression_store(ast, node, assembler, value_reg);
		break;

	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
		unary_operation_prefix_increment_decrement_store(ast, node, assembler, value_reg);
		break;
	case AstTag::UNARY_OPERATION_DEREFERENCE:
		unary_operation_dereference_store(ast, node, assembler, value_reg);
		break;
	default:
		err("ast_store - Unexpected AST node tag %hhu at %u:%u",
			ast.tags[node], ast.tokens[node].line, ast.tokens[node].col);
	}
}

#endif