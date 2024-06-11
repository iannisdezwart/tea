#ifndef TEA_AST_HEADER
#define TEA_AST_HEADER

#include <vector>
#include "Compiler/tokeniser.hpp"
#include "Compiler/type.hpp"
#include "Compiler/type-check/util.hpp"

enum struct SysCallId
{
	UNKNOWN,
	PRINT_CHAR,
	GET_CHAR,
};

enum class AstTag : uint8_t
{
	ASSIGNMENT_EXPRESSION_NORMAL,
	ASSIGNMENT_EXPRESSION_SUM,
	ASSIGNMENT_EXPRESSION_DIFFERENCE,
	ASSIGNMENT_EXPRESSION_PRODUCT,
	ASSIGNMENT_EXPRESSION_QUOTIENT,
	ASSIGNMENT_EXPRESSION_REMAINDER,
	ASSIGNMENT_EXPRESSION_LEFT_SHIFT,
	ASSIGNMENT_EXPRESSION_RIGHT_SHIFT,
	ASSIGNMENT_EXPRESSION_BITWISE_AND,
	ASSIGNMENT_EXPRESSION_BITWISE_OR,
	ASSIGNMENT_EXPRESSION_BITWISE_XOR,
	BINARY_OPERATION_ADDITION,
	BINARY_OPERATION_SUBTRACTION,
	BINARY_OPERATION_MULTIPLICATION,
	BINARY_OPERATION_DIVISION,
	BINARY_OPERATION_REMAINDER,
	BINARY_OPERATION_LEFT_SHIFT,
	BINARY_OPERATION_RIGHT_SHIFT,
	BINARY_OPERATION_BITWISE_AND,
	BINARY_OPERATION_BITWISE_OR,
	BINARY_OPERATION_BITWISE_XOR,
	BINARY_OPERATION_LESS,
	BINARY_OPERATION_LESS_OR_EQUAL,
	BINARY_OPERATION_GREATER,
	BINARY_OPERATION_GREATER_OR_EQUAL,
	BINARY_OPERATION_EQUAL,
	BINARY_OPERATION_NOT_EQUAL,
	BREAK_STATEMENT,
	CAST_EXPRESSION,
	CLASS_DECLARATION,
	CODE_BLOCK,
	CONTINUE_STATEMENT,
	FOR_STATEMENT,
	FUNCTION_CALL,
	FUNCTION_DECLARATION,
	IDENTIFIER_EXPRESSION_STANDALONE,
	IDENTIFIER_EXPRESSION_MEMBER,
	IF_STATEMENT,
	IF_ELSE_STATEMENT,
	LITERAL_CHAR_EXPRESSION,
	LITERAL_INTEGER_EXPRESSION,
	LITERAL_FLOAT_EXPRESSION,
	LITERAL_STRING_EXPRESSION,
	MEMBER_EXPRESSION_DIRECT,
	MEMBER_EXPRESSION_DIRECT_FINAL,
	MEMBER_EXPRESSION_DEREFERENCED,
	MEMBER_EXPRESSION_DEREFERENCED_FINAL,
	OFFSET_EXPRESSION,
	RETURN_VOID_STATEMENT,
	RETURN_EXPRESSION_STATEMENT,
	SYS_CALL,
	TYPE_IDENTIFIER_PAIR,
	TYPE_NAME,
	TYPE_NAME_INDIRECTION,
	UNARY_OPERATION_POSTFIX_INCREMENT,
	UNARY_OPERATION_POSTFIX_DECREMENT,
	UNARY_OPERATION_PREFIX_INCREMENT,
	UNARY_OPERATION_PREFIX_DECREMENT,
	UNARY_OPERATION_PLUS,
	UNARY_OPERATION_MINUS,
	UNARY_OPERATION_BITWISE_NOT,
	UNARY_OPERATION_LOGICAL_NOT,
	UNARY_OPERATION_DEREFERENCE,
	UNARY_OPERATION_ADDRESS_OF,
	VARIABLE_DECLARATION_UNINITIALISED,
	VARIABLE_DECLARATION_INITIALISED,
	WHILE_STATEMENT,
};

const char *
ast_tag_to_str(AstTag tag)
{
	switch (tag)
	{
	case AstTag::ASSIGNMENT_EXPRESSION_NORMAL:
		return "ASSIGNMENT_EXPRESSION_NORMAL";
	case AstTag::ASSIGNMENT_EXPRESSION_SUM:
		return "ASSIGNMENT_EXPRESSION_SUM";
	case AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE:
		return "ASSIGNMENT_EXPRESSION_DIFFERENCE";
	case AstTag::ASSIGNMENT_EXPRESSION_PRODUCT:
		return "ASSIGNMENT_EXPRESSION_PRODUCT";
	case AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT:
		return "ASSIGNMENT_EXPRESSION_QUOTIENT";
	case AstTag::ASSIGNMENT_EXPRESSION_REMAINDER:
		return "ASSIGNMENT_EXPRESSION_REMAINDER";
	case AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT:
		return "ASSIGNMENT_EXPRESSION_LEFT_SHIFT";
	case AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT:
		return "ASSIGNMENT_EXPRESSION_RIGHT_SHIFT";
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND:
		return "ASSIGNMENT_EXPRESSION_BITWISE_AND";
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR:
		return "ASSIGNMENT_EXPRESSION_BITWISE_OR";
	case AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR:
		return "ASSIGNMENT_EXPRESSION_BITWISE_XOR";
	case AstTag::BINARY_OPERATION_ADDITION:
		return "BINARY_OPERATION_ADDITION";
	case AstTag::BINARY_OPERATION_SUBTRACTION:
		return "BINARY_OPERATION_SUBTRACTION";
	case AstTag::BINARY_OPERATION_MULTIPLICATION:
		return "BINARY_OPERATION_MULTIPLICATION";
	case AstTag::BINARY_OPERATION_DIVISION:
		return "BINARY_OPERATION_DIVISION";
	case AstTag::BINARY_OPERATION_REMAINDER:
		return "BINARY_OPERATION_REMAINDER";
	case AstTag::BINARY_OPERATION_LEFT_SHIFT:
		return "BINARY_OPERATION_LEFT_SHIFT";
	case AstTag::BINARY_OPERATION_RIGHT_SHIFT:
		return "BINARY_OPERATION_RIGHT_SHIFT";
	case AstTag::BINARY_OPERATION_BITWISE_AND:
		return "BINARY_OPERATION_BITWISE_AND";
	case AstTag::BINARY_OPERATION_BITWISE_OR:
		return "BINARY_OPERATION_BITWISE_OR";
	case AstTag::BINARY_OPERATION_BITWISE_XOR:
		return "BINARY_OPERATION_BITWISE_XOR";
	case AstTag::BINARY_OPERATION_LESS:
		return "BINARY_OPERATION_LESS";
	case AstTag::BINARY_OPERATION_LESS_OR_EQUAL:
		return "BINARY_OPERATION_LESS_OR_EQUAL";
	case AstTag::BINARY_OPERATION_GREATER:
		return "BINARY_OPERATION_GREATER";
	case AstTag::BINARY_OPERATION_GREATER_OR_EQUAL:
		return "BINARY_OPERATION_GREATER_OR_EQUAL";
	case AstTag::BINARY_OPERATION_EQUAL:
		return "BINARY_OPERATION_EQUAL";
	case AstTag::BINARY_OPERATION_NOT_EQUAL:
		return "BINARY_OPERATION_NOT_EQUAL";
	case AstTag::BREAK_STATEMENT:
		return "BREAK_STATEMENT";
	case AstTag::CAST_EXPRESSION:
		return "CAST_EXPRESSION";
	case AstTag::CLASS_DECLARATION:
		return "CLASS_DECLARATION";
	case AstTag::CODE_BLOCK:
		return "CODE_BLOCK";
	case AstTag::CONTINUE_STATEMENT:
		return "CONTINUE_STATEMENT";
	case AstTag::FOR_STATEMENT:
		return "FOR_STATEMENT";
	case AstTag::FUNCTION_CALL:
		return "FUNCTION_CALL";
	case AstTag::FUNCTION_DECLARATION:
		return "FUNCTION_DECLARATION";
	case AstTag::IDENTIFIER_EXPRESSION_STANDALONE:
		return "IDENTIFIER_EXPRESSION_STANDALONE";
	case AstTag::IDENTIFIER_EXPRESSION_MEMBER:
		return "IDENTIFIER_EXPRESSION_MEMBER";
	case AstTag::IF_STATEMENT:
		return "IF_STATEMENT";
	case AstTag::IF_ELSE_STATEMENT:
		return "IF_ELSE_STATEMENT";
	case AstTag::LITERAL_CHAR_EXPRESSION:
		return "LITERAL_CHAR_EXPRESSION";
	case AstTag::LITERAL_INTEGER_EXPRESSION:
		return "LITERAL_INTEGER_EXPRESSION";
	case AstTag::LITERAL_FLOAT_EXPRESSION:
		return "LITERAL_FLOAT_EXPRESSION";
	case AstTag::LITERAL_STRING_EXPRESSION:
		return "LITERAL_STRING_EXPRESSION";
	case AstTag::MEMBER_EXPRESSION_DIRECT:
		return "MEMBER_EXPRESSION_DIRECT";
	case AstTag::MEMBER_EXPRESSION_DIRECT_FINAL:
		return "MEMBER_EXPRESSION_DIRECT_FINAL";
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED:
		return "MEMBER_EXPRESSION_DEREFERENCED";
	case AstTag::MEMBER_EXPRESSION_DEREFERENCED_FINAL:
		return "MEMBER_EXPRESSION_DEREFERENCED_FINAL";
	case AstTag::OFFSET_EXPRESSION:
		return "OFFSET_EXPRESSION";
	case AstTag::RETURN_VOID_STATEMENT:
		return "RETURN_VOID_STATEMENT";
	case AstTag::RETURN_EXPRESSION_STATEMENT:
		return "RETURN_EXPRESSION_STATEMENT";
	case AstTag::SYS_CALL:
		return "SYS_CALL";
	case AstTag::TYPE_IDENTIFIER_PAIR:
		return "TYPE_IDENTIFIER_PAIR";
	case AstTag::TYPE_NAME:
		return "TYPE_NAME";
	case AstTag::TYPE_NAME_INDIRECTION:
		return "TYPE_NAME_INDIRECTION";
	case AstTag::UNARY_OPERATION_POSTFIX_INCREMENT:
		return "UNARY_OPERATION_POSTFIX_INCREMENT";
	case AstTag::UNARY_OPERATION_POSTFIX_DECREMENT:
		return "UNARY_OPERATION_POSTFIX_DECREMENT";
	case AstTag::UNARY_OPERATION_PREFIX_INCREMENT:
		return "UNARY_OPERATION_PREFIX_INCREMENT";
	case AstTag::UNARY_OPERATION_PREFIX_DECREMENT:
		return "UNARY_OPERATION_PREFIX_DECREMENT";
	case AstTag::UNARY_OPERATION_PLUS:
		return "UNARY_OPERATION_UNARY_PLUS";
	case AstTag::UNARY_OPERATION_MINUS:
		return "UNARY_OPERATION_UNARY_MINUS";
	case AstTag::UNARY_OPERATION_BITWISE_NOT:
		return "UNARY_OPERATION_BITWISE_NOT";
	case AstTag::UNARY_OPERATION_LOGICAL_NOT:
		return "UNARY_OPERATION_LOGICAL_NOT";
	case AstTag::UNARY_OPERATION_DEREFERENCE:
		return "UNARY_OPERATION_DEREFERENCE";
	case AstTag::UNARY_OPERATION_ADDRESS_OF:
		return "UNARY_OPERATION_ADDRESS_OF";
	case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
		return "VARIABLE_DECLARATION_UNINITIALISED";
	case AstTag::VARIABLE_DECLARATION_INITIALISED:
		return "VARIABLE_DECLARATION_INITIALISED";
	case AstTag::WHILE_STATEMENT:
		return "WHILE_STATEMENT";
	default:
		return "UNKNOWN_TAG";
	}
}

union NodeData
{
	struct
	{
		uint store_node;
		uint value_node;
	} assignment_expression;

	struct
	{
		uint lhs_expr_node;
		uint rhs_expr_node;
		// cmp_type
	} binary_operation;

	struct
	{
	} break_statement;

	struct
	{
		uint expression_node;
		uint type_name_node;
	} cast_expression;

	struct
	{
		uint class_id;
		uint fields_ed_idx; // <len> + (len * <field_node>)
	} class_declaration;

	struct
	{
		uint statements_len;
		uint statements_ed_idx; // statements_len * <statement_node>
	} code_block;

	struct
	{
	} continue_statement;

	struct
	{
		uint init_node;
		uint rest_ed_idx; // <test_node> + <update_node> + <body_node>
	} for_statement;

	struct
	{
		uint callee_id;
		uint ed_idx; // <fn_signature_idx> + <args_len> + (args_len * <arg_node>)
	} function_call;

	struct
	{
		uint type_and_id_node;
		uint ed_idx; // <body_node> + <locals_size> + <params_len> + (params_len * <param_node>)
	} function_declaration;

	struct
	{
		uint identifier_id;
		int id_kind : 1;
		int offset : 31;
	} identifier_expression_standalone;

	struct
	{
		uint identifier_id;
		union
		{
			uint node;   // Used in type check phase.
			int offset; // Used in code generation phase.
		} object;
	} identifier_expression_member;

	struct
	{
		uint condition_node;
		uint then_block_node;
	} if_statement;

	struct
	{
		uint condition_node;
		uint ed_idx; // <then_block_node> + <else_block_node>
	} if_else_statement;

	struct
	{
		uint8_t value;
	} literal_char_expression;

	struct
	{
		uint64_t value;
	} literal_integer_expression;

	struct
	{
		double value;
	} literal_float_expression;

	struct
	{
		uint string_id;
	} literal_string_expression;

	struct
	{
		uint object_node;
		union
		{
			uint node;   // Used in type check phase.
			int offset; // Used in code generation phase.
		} member;
	} member_expression;

	struct
	{
		uint object_node;
		uint offset_node;
	} offset_expression;

	struct
	{
	} return_void_statement;

	struct
	{
		uint expression_node;
	} return_expression_statement;

	struct
	{
		SysCallId sys_call_id;
		uint arguments_ed_idx; // <len> + (len * <arg_node>)
	} sys_call;

	struct
	{
		uint type_node;
		uint identifier_id;
	} type_identifier_pair;

	struct
	{
		uint type_id;
	} type_name;

	struct
	{
		uint type_id;
		uint arr_sizes_ed_idx; // <len> + (len * <arr_size>)
	} type_name_indirection;

	struct
	{
		uint expression_node;
	} unary_operation;

	struct
	{
		uint type_and_id_node;
	} variable_declaration_uninitialised;

	struct
	{
		uint type_and_id_node;
		uint ed_idx; // <id_expr_node> + <assignment_node>
	} variable_declaration_initialised;

	struct
	{
		uint condition_node;
		uint body_node;
	} while_statement;
};

struct CompactToken
{
	uint line;
	uint col;

	CompactToken(const Token &token)
		: line(token.line), col(token.col) {}

	std::string
	to_str()
	{
		return std::to_string(line) + ":" + std::to_string(col);
	}
};

struct AST
{
	std::vector<AstTag> tags;
	std::vector<CompactToken> tokens;
	std::vector<NodeData> data;
	std::vector<uint> extra_data;
	std::vector<Type> types;
	std::vector<std::string> strings;
	std::vector<FunctionSignature> function_signatures;
	std::vector<uint> class_declarations;
	std::vector<uint> global_declarations;
	std::vector<uint> function_declarations;
};

#endif