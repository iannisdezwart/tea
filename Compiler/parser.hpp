#ifndef TEA_PARSER_HEADER
#define TEA_PARSER_HEADER

#include <vector>
#include "Compiler/tokeniser.hpp"
#include "Compiler/ASTNodes/TypeName.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/VariableDeclaration.hpp"
#include "Compiler/ASTNodes/FunctionDeclaration.hpp"
#include "Compiler/ASTNodes/ReturnStatement.hpp"
#include "Compiler/ASTNodes/CodeBlock.hpp"
#include "Compiler/ASTNodes/LiteralStringExpression.hpp"
#include "Compiler/ASTNodes/LiteralCharExpression.hpp"
#include "Compiler/ASTNodes/LiteralNumberExpression.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/ASTNodes/FunctionCall.hpp"
#include "Compiler/ASTNodes/BinaryOperation.hpp"
#include "Compiler/ASTNodes/UnaryOperation.hpp"
#include "Compiler/ASTNodes/MemberExpression.hpp"
#include "Compiler/ASTNodes/AssignmentExpression.hpp"
#include "Compiler/ASTNodes/IfStatement.hpp"
#include "Compiler/ASTNodes/WhileStatement.hpp"
#include "Compiler/ASTNodes/ForStatement.hpp"
#include "Compiler/ASTNodes/ClassDeclaration.hpp"
#include "Compiler/ASTNodes/CastExpression.hpp"
#include "Compiler/ASTNodes/OffsetExpression.hpp"
#include "Compiler/ASTNodes/SysCall.hpp"
#include "Compiler/ASTNodes/BreakStatement.hpp"
#include "Compiler/ASTNodes/ContinueStatement.hpp"

/**
 * @brief Merges binary operators with left-to-right associativity.
 */
void
merge_bin_ops_ltr(AST &ast, std::vector<uint> &expressions,
	std::vector<Token> &operators, const std::vector<Operator> &active_ops)
{
	for (size_t j = 0; j < operators.size(); j++)
	{
		Token &op_token = operators[j];
		Operator op     = str_to_operator(op_token.value);

		// Check if this operator is mergeable.

		for (size_t k = 0; k < active_ops.size(); k++)
		{
			if (active_ops[k] == op)
				goto merge_op;
		}

		// It is not mergeable, check the next operator of the
		// compound expression.

		continue;

		// It is mergeable into a BinaryOperation, so merge it.

	merge_op:

		uint left_expr  = expressions[j];
		uint right_expr = expressions[j + 1];
		uint new_expr   = ast.data.size();

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case ADDITION:
			ast.tags.push_back(AstTag::BINARY_OPERATION_ADDITION);
			goto binop;
		case SUBTRACTION:
			ast.tags.push_back(AstTag::BINARY_OPERATION_SUBTRACTION);
			goto binop;
		case MULTIPLICATION:
			ast.tags.push_back(AstTag::BINARY_OPERATION_MULTIPLICATION);
			goto binop;
		case DIVISION:
			ast.tags.push_back(AstTag::BINARY_OPERATION_DIVISION);
			goto binop;
		case REMAINDER:
			ast.tags.push_back(AstTag::BINARY_OPERATION_REMAINDER);
			goto binop;
		case BITWISE_AND:
			ast.tags.push_back(AstTag::BINARY_OPERATION_BITWISE_AND);
			goto binop;
		case BITWISE_XOR:
			ast.tags.push_back(AstTag::BINARY_OPERATION_BITWISE_XOR);
			goto binop;
		case BITWISE_OR:
			ast.tags.push_back(AstTag::BINARY_OPERATION_BITWISE_OR);
			goto binop;
		case LEFT_SHIFT:
			ast.tags.push_back(AstTag::BINARY_OPERATION_LEFT_SHIFT);
			goto binop;
		case RIGHT_SHIFT:
			ast.tags.push_back(AstTag::BINARY_OPERATION_RIGHT_SHIFT);
			goto binop;
		case LESS:
			ast.tags.push_back(AstTag::BINARY_OPERATION_LESS);
			goto binop;
		case LESS_OR_EQUAL:
			ast.tags.push_back(AstTag::BINARY_OPERATION_LESS_OR_EQUAL);
			goto binop;
		case GREATER:
			ast.tags.push_back(AstTag::BINARY_OPERATION_GREATER);
			goto binop;
		case GREATER_OR_EQUAL:
			ast.tags.push_back(AstTag::BINARY_OPERATION_GREATER_OR_EQUAL);
			goto binop;
		case EQUAL:
			ast.tags.push_back(AstTag::BINARY_OPERATION_EQUAL);
			goto binop;
		case NOT_EQUAL:
			ast.tags.push_back(AstTag::BINARY_OPERATION_NOT_EQUAL);
			goto binop;
		case POINTER_TO_MEMBER:
			ast.tags.push_back(AstTag::MEMBER_EXPRESSION_DIRECT); // TODO: FINAL
			goto membexpr;
		case DEREFERENCED_POINTER_TO_MEMBER:
			ast.tags.push_back(AstTag::MEMBER_EXPRESSION_DEREFERENCED); // TODO: FINAL
			goto membexpr;
		}

	binop:
		ast.data.push_back(NodeData {
			.binary_operation = {
				.lhs_expr_node = left_expr,
				.rhs_expr_node = right_expr,
			},
		});
		goto end;

	membexpr:

		if (ast.tags[right_expr] != AstTag::IDENTIFIER_EXPRESSION_STANDALONE)
		{
			err_at_token(ast.tokens[right_expr], "Type Error",
				"A member of a class instance must be an IdentifierExpression "
				"or a FunctionCall\n"
				"Found a %s",
				ast_tag_to_str(ast.tags[right_expr]));
		}

		ast.data.push_back(NodeData {
			.member_expression = {
				.object_node = left_expr,
				.member {
					.node = right_expr,
				},
			},
		});
		goto end;

	end:

		ast.tokens.push_back(CompactToken(op_token));

		expressions[j] = new_expr;

		operators.erase(operators.begin() + j);
		expressions.erase(expressions.begin() + j + 1);

		j--;
	}
}

/**
 * @brief Merges binary operators with right-to-left associativity.
 */
void
merge_bin_ops_rtl(AST &ast, std::vector<uint> &expressions,
	std::vector<Token> &operators, const std::vector<Operator> &active_ops)
{
	for (size_t j = operators.size(); j != 0; j--)
	{
		Token &op_token = operators[j - 1];
		Operator op     = str_to_operator(op_token.value);

		// Check if this operator is mergeable.

		for (size_t k = 0; k < active_ops.size(); k++)
		{
			if (active_ops[k] == op)
				goto merge_op;
		}

		// It is not mergeable, check the next operator of the
		// compound expression.

		continue;

		// It is mergeable into a BinaryOperation, so merge it.

	merge_op:

		uint left     = expressions[j - 1];
		uint right    = expressions[j];
		uint new_expr = ast.data.size();

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_NORMAL);
			break;
		case SUM_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_SUM);
			break;
		case DIFFERENCE_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_DIFFERENCE);
			break;
		case PRODUCT_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_PRODUCT);
			break;
		case QUOTIENT_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_QUOTIENT);
			break;
		case REMAINDER_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_REMAINDER);
			break;
		case LEFT_SHIFT_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_LEFT_SHIFT);
			break;
		case RIGHT_SHIFT_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_RIGHT_SHIFT);
			break;
		case BITWISE_AND_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_BITWISE_AND);
			break;
		case BITWISE_XOR_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_BITWISE_XOR);
			break;
		case BITWISE_OR_ASSIGNMENT:
			ast.tags.push_back(AstTag::ASSIGNMENT_EXPRESSION_BITWISE_OR);
			break;
		}

		ast.data.push_back(NodeData {
			.assignment_expression = {
				.store_node = left,
				.value_node = right,
			},
		});
		ast.tokens.push_back(CompactToken(op_token));

		expressions[j - 1] = new_expr;

		operators.erase(operators.begin() + j - 1);
		expressions.erase(expressions.begin() + j);
	}
}

/**
 * Merges unary operators with left-to-right associativity.
 */
void
merge_un_ops_ltr(AST &ast, uint &expression,
	std::vector<std::pair<Token, bool>> &operators,
	const std::vector<Operator> &active_ops)
{
	for (size_t j = 0; j < operators.size(); j++)
	{
		Token &op_token = operators[j].first;
		bool prefix     = operators[j].second;
		Operator op     = str_to_operator(op_token.value, prefix);

		// Check if this operator is mergeable.

		for (size_t k = 0; k < active_ops.size(); k++)
		{
			if (active_ops[k] == op)
			{
				goto merge_op;
			}
		}

		// It is not mergeable, check the next operator of the
		// compound unary expression.

		continue;

		// It is mergeable into a UnaryOperation, so merge it.

	merge_op:

		uint new_expr = ast.data.size();

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case POSTFIX_INCREMENT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_POSTFIX_INCREMENT);
			break;
		case POSTFIX_DECREMENT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_POSTFIX_DECREMENT);
			break;
		}

		ast.data.push_back(NodeData {
			.unary_operation = {
				.expression_node = expression,
			},
		});
		ast.tokens.push_back(CompactToken(op_token));

		expression = new_expr;

		operators.erase(operators.begin() + j);

		j--;
	}
}

/**
 * @brief Merges unary operators with right-to-left associativity.
 */
void
merge_un_ops_rtl(AST &ast, uint &expression,
	std::vector<std::pair<Token, bool>> &operators,
	const std::vector<Operator> &active_ops)
{
	for (size_t j = operators.size(); j != 0; j--)
	{
		Token &op_token = operators[j - 1].first;
		bool prefix     = operators[j - 1].second;
		Operator op     = str_to_operator(op_token.value, prefix);

		// Check if this operator is mergeable.

		for (size_t k = 0; k < active_ops.size(); k++)
		{
			if (active_ops[k] == op)
			{
				goto merge_op_1;
			}
		}

		// It is not mergeable, check the next operator of the
		// compound unary expression.

		continue;

		// It is mergeable into a UnaryOperation, so merge it.

	merge_op_1:

		uint new_expr = ast.data.size();

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case PREFIX_INCREMENT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_PREFIX_INCREMENT);
			break;
		case PREFIX_DECREMENT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_PREFIX_DECREMENT);
			break;
		case UNARY_PLUS:
			ast.tags.push_back(AstTag::UNARY_OPERATION_PLUS);
			break;
		case UNARY_MINUS:
			ast.tags.push_back(AstTag::UNARY_OPERATION_MINUS);
			break;
		case BITWISE_NOT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_BITWISE_NOT);
			break;
		case LOGICAL_NOT:
			ast.tags.push_back(AstTag::UNARY_OPERATION_LOGICAL_NOT);
			break;
		case DEREFERENCE:
			ast.tags.push_back(AstTag::UNARY_OPERATION_DEREFERENCE);
			break;
		case ADDRESS_OF:
			ast.tags.push_back(AstTag::UNARY_OPERATION_ADDRESS_OF);
			break;
		}

		ast.data.push_back(NodeData {
			.unary_operation = {
				.expression_node = expression,
			},
		});

		ast.tokens.push_back(CompactToken(op_token));

		expression = new_expr;

		operators.erase(operators.begin() + j - 1);
	}
}

/**
 * @brief Parser class. Parses the tokenised input and builds an AST.
 * First, the input file has to be tokenised by the `Tokeniser` class.
 */
struct Parser
{
	// Reference to the input tokens
	// that were generated by the tokeniser.
	std::vector<Token> &tokens;

	// The index of the current token being parsed.
	size_t i = 0;

	// Mapping of identifier names to IDs.
	std::unordered_set<std::string> class_names;
	std::unordered_map<std::string, uint32_t> name_to_id;
	std::unordered_map<uint32_t, std::string> id_to_name;

/**
 * @brief Macro that pretty-prints a syntax error.
 * @param token The token that caused the error.
 * @param message The error message format string.
 * @param ... The format string arguments.
 */
#define syntax_err(token, message, ...)                                           \
	do                                                                        \
	{                                                                         \
		p_warn(stderr, "[ Syntax Error ]: " message "\n", ##__VA_ARGS__); \
		p_warn(stderr, "At %ld:%ld\n", token.line, token.col);            \
		abort();                                                          \
	} while (0)

/**
 * @brief Macro that pretty-prints an unexpected token error.
 * @param token The unexpected token.
 */
#define unexpected_token_syntax_err(token)                  \
	syntax_err(token, "Unexpected token %s of type %s", \
		token.value.c_str(), token_type_to_str(token.type));

/**
 * @brief Macro that asserts that the current token is of
 * an expected type. If the token is not of the expected type,
 * a syntax error is thrown.
 * @param token The token to check the type of.
 * @param token_type The expected token type.
 * TODO: Add extra optional arguments to add a message
 * if the token does not have the correct type.
 */
#define assert_token_type(token, token_type)                                                   \
	do                                                                                     \
	{                                                                                      \
		if (token.type != token_type)                                                  \
		{                                                                              \
			p_warn(stderr,                                                         \
				"[ Syntax Error ]: Unexpected token of type %s, "              \
				"expected a %s token.\n",                                      \
				token_type_to_str(token.type), token_type_to_str(token_type)); \
			p_warn(stderr, "At %ld:%ld\n", token.line, token.col);                 \
			abort();                                                               \
		}                                                                              \
	} while (0)

/**
 * @brief Macro that asserts that the current token holds a
 * given value. If the token does not have the given value,
 * a syntax error is thrown.
 * @param token The token to check the value of.
 * @param token_value The expected token value.
 * TODO: Add extra optional arguments to add a message
 * if the token does not have the correct value.
 */
#define assert_token_value(token, token_value)                                           \
	do                                                                               \
	{                                                                                \
		if (token.value != token_value)                                          \
		{                                                                        \
			p_warn(stderr,                                                   \
				"[ Syntax Error ]: Unexpected token with value \"%s\", " \
				"expected a token with value \"%s\".\n",                 \
				token.value.c_str(), token_value);                       \
			p_warn(stderr, "At %ld:%ld\n", token.line, token.col);           \
			abort();                                                         \
		}                                                                        \
	} while (0)

	/**
	 * @brief Constructs a new Parser object.
	 * @param tokens The tokens generated by the `Tokeniser` class.
	 */
	Parser(std::vector<Token> &tokens)
		: tokens(tokens) {}

	/**
	 * @brief Parses the input tokens and builds an AST.
	 * @returns The root nodes of all function declarations,
	 * class declarations and global variable declarations,
	 * in order of definition within the source code file.
	 */
	std::tuple<
		AST,
		std::unordered_map<uint32_t, std::string>,
		std::unordered_map<std::string, uint32_t>>
	parse()
	{
		AST ast;

		scan_class_names();

		while (i < tokens.size())
		{
			uint next_statement_node = next_statement(ast);

			switch (ast.tags[next_statement_node])
			{
			case AstTag::CLASS_DECLARATION:
				ast.class_declarations.push_back(next_statement_node);
				break;
			case AstTag::VARIABLE_DECLARATION_UNINITIALISED:
			case AstTag::VARIABLE_DECLARATION_INITIALISED: // TODO: Is okay?
				ast.global_declarations.push_back(next_statement_node);
				break;
			case AstTag::FUNCTION_DECLARATION:
				ast.function_declarations.push_back(next_statement_node);
				break;
			default:
				err("Unexpected top level statement with tag %s\n",
					ast_tag_to_str(ast.tags[next_statement_node]));
			}
		}

		ast.types.resize(ast.data.size());
		return std::make_tuple(std::move(ast), std::move(id_to_name), std::move(name_to_id));
	}

	/**
	 * @brief Scans all class names in the input tokens.
	 * This is needed to check whether an identifier is a type name.
	 */
	void
	scan_class_names()
	{
		for (size_t j = 0; j < tokens.size(); j++)
		{
			Token token = tokens[j];

			if (token.type == KEYWORD && token.value == "class")
			{
				Token class_name_token = tokens[j + 1];
				assert_token_type(class_name_token, IDENTIFIER);

				uint32_t class_id = name_to_id.size() + BUILTIN_TYPE_END;
				class_names.insert(class_name_token.value);
				name_to_id[class_name_token.value] = class_id;
				id_to_name[class_id]               = class_name_token.value;
			}
		}
	}

	/**
	 * @brief Gets the token at a specific index.
	 * @param offset The offset to the token. Defaults to 0.
	 * @returns The token at the offset.
	 */
	Token
	get_token(size_t offset = 0)
	{
		if (i + offset >= tokens.size())
		{
			p_warn(stderr, "[ Parser Error ]: Unexpected end of file.\n");
			abort();
		}

		Token token = tokens[i + offset];

		if (token.type == IDENTIFIER)
		{
			if (class_names.find(token.value) != class_names.end())
			{
				// User defined class
				token.type = TYPE;
			}
			else
			{
				//
			}
		}

		return token;
	}

	/**
	 * @returns The next token.
	 * The current token index is incremented.
	 */
	Token
	next_token()
	{
		Token token = get_token();
		i++;
		return token;
	}

	uint
	register_identifier(const std::string &id_str)
	{
		auto it = name_to_id.find(id_str);
		uint identifier_id;
		if (it == name_to_id.end())
		{
			identifier_id             = name_to_id.size() + BUILTIN_TYPE_END;
			name_to_id[id_str]        = identifier_id;
			id_to_name[identifier_id] = id_str;
		}
		else
		{
			identifier_id = it->second;
		}

		return identifier_id;
	}

	/**
	 * @brief Looks at the next couple of tokens and figures out
	 * what kind of statement they represent. The statement
	 * is then parsed and returned as an AST node.
	 */
	uint
	next_statement(AST &ast)
	{
		Token token = get_token();

		// Look at the first token and figure out what
		// statement this could be. If this is ambiguous,
		// the next tokens are also checked.
		// As soon as the parser knows what kind of statement
		// this is, the statement is scanned and returned
		// as an AST node. If the parser cannot figure out
		// what is going on, a syntax error is thrown.

		switch (token.type)
		{
		case TYPE:
			// TODO: think about what should happen
			// when using the casting syntax
			// `u8(x)` on a statement level.
			// Currently, this will throw a syntax
			// error, because the type comes first
			// and a declaration is expected.

			return scan_declaration(ast);

		case KEYWORD:
			if (token.value == "return")
			{
				uint node = scan_return_statement(ast);
				expect_statement_terminator();
				return node;
			}

			if (token.value == "if")
			{
				return scan_if_statement(ast);
			}

			if (token.value == "while")
			{
				return scan_while_statement(ast);
			}

			if (token.value == "for")
			{
				return scan_for_statement(ast);
			}

			if (token.value == "class")
			{
				return scan_class_declaration(ast);
			}

			if (token.value == "syscall")
			{
				return scan_syscall(ast);
			}

			if (token.value == "break")
			{
				return scan_break_statement(ast);
			}

			if (token.value == "continue")
			{
				return scan_continue_statement(ast);
			}

			else
				err("[ Parser Error ]: Keyword %s not handled.\n",
					token.value.c_str());

		default:
		{
			uint node = scan_expression(ast);
			expect_statement_terminator();
			return node;
		}
		}
	}

	/**
	 * @brief Checks if the next token is a semicolon.
	 */
	void
	expect_statement_terminator()
	{
		Token terminator = next_token();
		assert_token_type(terminator, SPECIAL_CHARACTER);
		assert_token_value(terminator, ";");
	}

	/**
	 * Scans a type name:
	 * <primitive_type_name>(any number of "*" or "[<number>]")
	 *
	 * Example of valid type names:
	 * * u8
	 * * u8[10]
	 * * u8*
	 * * u8*[10]****[7]**
	 */
	uint
	scan_type_name(AST &ast)
	{
		Token type_name_token = next_token();
		assert_token_type(type_name_token, TYPE);

		// TODO: add generics and references maybe

		std::vector<uint> array_sizes;
		Token token;

		// Scan all pointer and array specifiers.

		while (true)
		{
			token = get_token();

			// Array specifier

			if (token.type == SPECIAL_CHARACTER && token.value == "[")
			{
				i++;

				Token size_token = next_token();
				assert_token_type(size_token, LITERAL_NUMBER);
				array_sizes.push_back(stoul(size_token.value));

				Token array_size_end_token = next_token();
				assert_token_type(array_size_end_token, SPECIAL_CHARACTER);
				assert_token_value(array_size_end_token, "]");
			}

			// Pointer

			else if (token.type == OPERATOR && token.value == "*")
			{
				i++;

				array_sizes.push_back(0);
			}

			else
				break;
		}

		uint type_id = class_names.find(type_name_token.value) != class_names.end()
			? name_to_id[type_name_token.value]
			: builtin_type_from_string(type_name_token.value);

		if (array_sizes.empty())
		{
			uint type_name_node = ast.data.size();
			ast.tags.push_back(AstTag::TYPE_NAME);
			ast.data.push_back(NodeData {
				.type_name = {
					.type_id = type_id,
				},
			});
			ast.tokens.push_back(CompactToken(type_name_token));
			return type_name_node;
		}

		uint arr_sizes_ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(array_sizes.size());
		ast.extra_data.insert(ast.extra_data.end(), array_sizes.begin(), array_sizes.end());

		uint type_name_node = ast.data.size();
		ast.tags.push_back(AstTag::TYPE_NAME_INDIRECTION);
		ast.data.push_back(NodeData {
			.type_name_indirection = {
				.type_id          = type_id,
				.arr_sizes_ed_idx = arr_sizes_ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(type_name_token));
		return type_name_node;
	}

	/**
	 * Scans a pair of a type and an identifier.
	 * <type> <identifier_name>
	 *
	 * Useful when scanning a declaration.
	 */
	uint
	scan_type_identifier_pair(AST &ast)
	{
		uint type_name = scan_type_name(ast);

		Token identifier_token = next_token();
		assert_token_type(identifier_token, IDENTIFIER);

		uint identifier_id = register_identifier(identifier_token.value);

		uint type_identifier_node = ast.data.size();
		ast.tags.push_back(AstTag::TYPE_IDENTIFIER_PAIR);
		ast.data.push_back(NodeData {
			.type_identifier_pair = {
				.type_node     = type_name,
				.identifier_id = identifier_id,
			},
		});
		ast.tokens.push_back(CompactToken(identifier_token));
		return type_identifier_node;
	}

	/**
	 * Scans a code block. Note that in the Tea language
	 * a single statement is also allowed instead of a code block.
	 * This is handled by the `scan_code_block_or_statement()`
	 * method.
	 * "{ any number of <statement> }"
	 */
	uint
	scan_code_block(AST &ast)
	{
		// Expect a left curly brace "{".

		Token curly_brace_start = next_token();
		assert_token_type(curly_brace_start, SPECIAL_CHARACTER);
		assert_token_value(curly_brace_start, "{");

		std::vector<uint> statements;

		// Scan all statements.

		while (true)
		{
			Token maybe_curly_brace_end = get_token();

			if (maybe_curly_brace_end.type == SPECIAL_CHARACTER
				&& maybe_curly_brace_end.value == "}")
			{
				// End of code block, consume the
				// right curly brace "}".

				i++;
				break;
			}

			statements.push_back(next_statement(ast));
		}

		uint statements_ed_idx = ast.extra_data.size();
		ast.extra_data.insert(ast.extra_data.end(), statements.begin(), statements.end());

		uint code_block_node = ast.data.size();
		ast.tags.push_back(AstTag::CODE_BLOCK);
		ast.data.push_back(NodeData {
			.code_block = {
				.statements_len    = static_cast<uint>(statements.size()),
				.statements_ed_idx = statements_ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(curly_brace_start));

		return code_block_node;
	}

	/**
	 * Scans a single statement and wraps it inside a code block.
	 * Used by the `scan_code_block_or_statement()` method,
	 * to support single line statements instead of code blocks.
	 * <statement>
	 */
	uint
	scan_and_wrap_statement_inside_code_block(AST &ast)
	{
		// Scan the statement.

		Token first_token = get_token();
		uint statement    = next_statement(ast);

		// Wrap it inside a code block.

		uint statement_ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(statement);

		uint code_block_node = ast.data.size();
		ast.tags.push_back(AstTag::CODE_BLOCK);
		ast.data.push_back(NodeData {
			.code_block = {
				.statements_len    = 1,
				.statements_ed_idx = statement_ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(first_token));

		return code_block_node;
	}

	/**
	 * Scans either a code block, or a single statement.
	 * <code_block> or <statement>
	 *
	 * Used to scan a statement or a code block that comes
	 * after an if, while, for etc.
	 */
	uint
	scan_code_block_or_statement(AST &ast)
	{
		Token first_token = get_token();

		// Check if there is an opening curly brace

		if (first_token.type == SPECIAL_CHARACTER && first_token.value == "{")
		{
			// If so, scan the code block

			return scan_code_block(ast);
		}
		else
		{
			// Else, scan the statement and wrap it inside a code block

			return scan_and_wrap_statement_inside_code_block(ast);
		}
	}

	/**
	 * Scans any expression. If the expression is compound, this
	 * method is recursively called and the correct operator
	 * precedence is enforced.
	 *
	 * Examples of expressions this method can scan:
	 * * x
	 * * x + y
	 * * (x * y / z - a + b & c << (d >> e))
	 */
	uint
	scan_expression(AST &ast)
	{
		// List of the expressions
		std::vector<uint> expressions;

		// List of all operators between the expressions
		std::vector<Token> operators;

		// Scans all sub expressions and operators that
		// connect them.
		// This loop stops when the next token is not
		// an operator.

		while (true)
		{
			expressions.push_back(scan_sub_expression(ast));

			// Check if the next token is an operator.
			// If it isn't, we're done.

			Token maybe_operator_token = get_token();
			if (maybe_operator_token.type != OPERATOR)
				break;

			// It is an operator, push its token to the operator vector.

			i++;
			operators.push_back(maybe_operator_token);
		}

		// Go through all existing operators, descending in precedence.
		// This loop will merge all expressions that are connected
		// by an operator and enforces the correct operator
		// precedence.

		for (size_t i = 0; i < operator_precedence.size(); i++)
		{
			const OperatorPrecedencePair &o_p_pair  = operator_precedence[i];
			const std::vector<Operator> &active_ops = o_p_pair.first;
			const Associativity &associativity      = o_p_pair.second;

			if (associativity == LEFT_TO_RIGHT)
			{
				merge_bin_ops_ltr(ast, expressions, operators, active_ops);
			}
			else
			{
				merge_bin_ops_rtl(ast, expressions, operators, active_ops);
			}
		}

		uint expression = std::move(expressions[0]);

		// OffsetExpression

		for (Token next = get_token();
			next.type == SPECIAL_CHARACTER && next.value == "[";
			next = get_token())
		{
			expression = scan_offset_expression(ast, expression);
		}

		return expression;
	}

	/**
	 * Scans a sub-expression inside an expression.
	 * This is essentially a single expression, or a compound
	 * expression that is wrapped in parentheses.
	 *
	 * Examples of sub-expressions:
	 * * x
	 * * !x
	 * * (x + y)
	 * * (x + y >> b << (c / d))
	 *
	 * Examples of non-sub-expressions
	 * (cannot be parsed as a sub-expression):
	 * * (x + y) + z
	 * * a + *b
	 */
	uint
	scan_sub_expression(AST &ast)
	{
		std::vector<std::pair<Token, bool>> operators; // bool => true = prefix
		uint expression;

		// Prefix unary operators

		while (true)
		{
			// Check if the next token is an operator

			Token maybe_operator_token = get_token();
			if (maybe_operator_token.type != OPERATOR)
			{
				break;
			}

			Operator op = str_to_operator(maybe_operator_token.value, true);
			if (!is_prefix_unary_operator(op))
			{
				break;
			}

			// It is a prefix unary operator, push its token to the operator vector

			i++;
			operators.push_back(std::make_pair(maybe_operator_token, true));
		}

		Token expr_token = next_token();

		// Casting
		// <type>(<expr>)

		if (expr_token.type == TYPE)
		{
			Token left_parenthesis = next_token();
			assert_token_type(left_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis, "(");

			uint type_name_node = ast.data.size();
			ast.tags.push_back(AstTag::TYPE_NAME);
			ast.data.push_back(NodeData {
				.type_name = {
					.type_id = class_names.find(expr_token.value) != class_names.end()
						? name_to_id[expr_token.value]
						: builtin_type_from_string(expr_token.value),
				},
			});
			ast.tokens.push_back(CompactToken(expr_token));

			uint expr_node = scan_expression(ast);

			expression = ast.data.size();
			ast.tags.push_back(AstTag::CAST_EXPRESSION);
			ast.data.push_back(NodeData {
				.cast_expression = {
					.type_name_node  = type_name_node,
					.expression_node = expr_node,

				},
			});
			ast.tokens.push_back(CompactToken(left_parenthesis));

			Token right_parenthesis = next_token();
			assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis, ")");
		}

		// Casting
		// [<type> <*>](<expr>)
		// Maybe this is unnecessary. My idea is that we include
		// this syntax because things like `u8*[10](<expr>)`
		// might be unreadable.
		// Is `[ u8*[10] ](<expr>)` really more readable?
		// TODO: make a decision.

		else if (expr_token.type == SPECIAL_CHARACTER && expr_token.value == "[")
		{
			uint type_name_node = scan_type_name(ast);

			Token right_square_bracket = next_token();
			assert_token_type(right_square_bracket, SPECIAL_CHARACTER);
			assert_token_value(right_square_bracket, "]");

			Token left_parenthesis = next_token();
			assert_token_type(left_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis, "(");

			uint expr_node = scan_expression(ast);

			expression = ast.data.size();
			ast.tags.push_back(AstTag::CAST_EXPRESSION);
			ast.data.push_back(NodeData {
				.cast_expression = {
					.type_name_node  = type_name_node,
					.expression_node = expr_node,

				},
			});
			ast.tokens.push_back(CompactToken(left_parenthesis));

			Token right_parenthesis = next_token();
			assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis, ")");
		}

		// Parenthesised Expression
		// (<expr>)

		else if (expr_token.type == SPECIAL_CHARACTER && expr_token.value == "(")
		{
			expression = scan_expression(ast);

			// Expect right parenthesis

			Token right_parenthesis = next_token();
			assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis, ")");
		}

		// Literal string

		else if (expr_token.type == LITERAL_STRING)
		{
			uint string_id = ast.strings.size();
			ast.strings.push_back(expr_token.value);

			expression = ast.data.size();
			ast.tags.push_back(AstTag::LITERAL_STRING_EXPRESSION);
			ast.data.push_back(NodeData {
				.literal_string_expression = {
					.string_id = string_id,
				},
			});
			ast.tokens.push_back(CompactToken(expr_token));

			// TODO: allow multiple literal strings next to each other.
		}

		// Literal char

		else if (expr_token.type == LITERAL_CHAR)
		{
			expression = ast.data.size();
			ast.tags.push_back(AstTag::LITERAL_CHAR_EXPRESSION);
			ast.data.push_back(NodeData {
				.literal_char_expression = {
					.value = static_cast<uint8_t>(expr_token.value[0]),
				},
			});
			ast.tokens.push_back(CompactToken(expr_token));

			// TODO: make a method that parses a char correctly.
			// Currently, it will just parse the first char.
		}

		// Literal number

		else if (expr_token.type == LITERAL_NUMBER)
		{
			expression = ast.data.size();

			if (expr_token.value.find('.') != std::string::npos)
			{
				ast.tags.push_back(AstTag::LITERAL_FLOAT_EXPRESSION);
				ast.data.push_back(NodeData {
					.literal_float_expression = {
						.value = std::stod(expr_token.value),
					},
				});
			}
			else
			{
				uint64_t val;
				if (expr_token.value[0] == '0' && expr_token.value[1] == 'x')
					val = std::stoul(expr_token.value, nullptr, 16);
				else if (expr_token.value[0] == '0' && expr_token.value[1] == 'b')
					val = std::stoul(expr_token.value, nullptr, 2);
				// TODO: Support more cases.
				else
					val = std::stoul(expr_token.value);

				ast.tags.push_back(AstTag::LITERAL_INTEGER_EXPRESSION);
				ast.data.push_back(NodeData {
					.literal_integer_expression = {
						.value = val,
					},
				});
			}

			ast.tokens.push_back(CompactToken(expr_token));
		}

		// IdentifierExpression or FunctionCall

		else if (expr_token.type == IDENTIFIER)
		{
			Token next = get_token();

			// FunctionCall

			if (next.type == SPECIAL_CHARACTER && next.value == "(")
			{
				i--;
				expression = scan_function_call(ast);
			}

			// IdentifierExpression

			else
			{
				uint identifier_id = register_identifier(expr_token.value);

				expression = ast.data.size();
				ast.tags.push_back(AstTag::IDENTIFIER_EXPRESSION_STANDALONE);
				ast.data.push_back(NodeData {
					.identifier_expression_standalone = {
						.identifier_id = identifier_id,
					},
				});
				ast.tokens.push_back(CompactToken(expr_token));
			}
		}

		// Unexpected token

		else
		{
			unexpected_token_syntax_err(expr_token);
		}

		// Postfix unary operators

		while (true)
		{
			// Check if the next token is an operator.

			Token maybe_operator_token = get_token();
			if (maybe_operator_token.type != OPERATOR)
			{
				break;
			}

			Operator op = str_to_operator(maybe_operator_token.value);
			if (!is_postfix_unary_operator(op))
			{
				break;
			}

			// It is a postfix unary operator, push its token to the operator vector.

			i++;
			operators.push_back(std::make_pair(maybe_operator_token, false));
		}

		// Go through all existing operators, descending in precedence.

		for (size_t i = 0; i < operator_precedence.size(); i++)
		{
			const OperatorPrecedencePair &o_p_pair  = operator_precedence[i];
			const std::vector<Operator> &active_ops = o_p_pair.first;
			const Associativity &associativity      = o_p_pair.second;

			if (associativity == LEFT_TO_RIGHT)
			{
				merge_un_ops_ltr(ast, expression, operators, active_ops);
			}
			else
			{
				merge_un_ops_rtl(ast, expression, operators, active_ops);
			}
		}

		return expression;
	}

	/**
	 * Scans a declaration. This can either be a function
	 * declaration or a variable declaration.
	 */
	uint
	scan_declaration(AST &ast)
	{
		uint type_id_pair = scan_type_identifier_pair(ast);

		Token maybe_left_parenthesis = next_token();

		if (maybe_left_parenthesis.type == SPECIAL_CHARACTER
			&& maybe_left_parenthesis.value == "(")
		{
			// This is a function declaration

			return scan_function_declaration(ast, type_id_pair);
		}

		// This is a variable declaration

		i--;
		uint var_decl = scan_variable_declaration(ast, type_id_pair);
		expect_statement_terminator();
		return var_decl;
	}

	/**
	 * Scans a function declaration.
	 * <type_id_pair> <identifier>(<parameter_list>) <code_block>
	 *
	 * @param type_id_pair The type identifier pair of the function.
	 * This was parsed by the `scan_declaration()` function.
	 */
	uint
	scan_function_declaration(AST &ast, uint type_id_pair)
	{
		std::vector<uint> params;
		Token next = get_token();

		// Check if there are no parameters.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_parameters;
		}

		// Scan parameters.

	next_parameter:

		params.push_back(scan_type_identifier_pair(ast));
		next = next_token();

		if (next.type != SPECIAL_CHARACTER)
		{
			goto param_err;
		}

		if (next.value == ")")
		{
			goto end_parameters;
		}

		if (next.value == ",")
		{
			goto next_parameter;
		}

	param_err:

		unexpected_token_syntax_err(next);

	end_parameters:

		// Scan the function body.

		uint code_block = scan_code_block(ast);

		uint ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(code_block);
		ast.extra_data.push_back(0); // locals_size, will be filled during type checking.
		ast.extra_data.push_back(params.size());
		ast.extra_data.insert(ast.extra_data.end(), params.begin(), params.end());

		uint function_decl_node = ast.data.size();
		ast.tags.push_back(AstTag::FUNCTION_DECLARATION);
		ast.data.push_back(NodeData {
			.function_declaration = {
				.type_and_id_node = type_id_pair,
				.ed_idx           = ed_idx,
			},
		});
		ast.tokens.push_back(ast.tokens[type_id_pair]);
		return function_decl_node;
	}

	/**
	 * Scans a function body.
	 * <identifier>(<parameter_list>)
	 */
	uint
	scan_function_call(AST &ast)
	{
		// Scan the function name.

		Token identifier_token = next_token();
		assert_token_type(identifier_token, IDENTIFIER);

		// Consume the left paranthesis "(".

		Token left_parenthesis_token = next_token();
		assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(left_parenthesis_token, "(");

		std::vector<uint> arguments;
		Token next = get_token();

		// Check if there are no arguments.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_arguments;
		}

		// Scan the arguments.

	next_argument:

		arguments.push_back(scan_expression(ast));
		next = next_token();

		if (next.type != SPECIAL_CHARACTER)
		{
			goto argument_err;
		}

		if (next.value == ")")
		{
			goto end_arguments;
		}

		if (next.value == ",")
		{
			goto next_argument;
		}

	argument_err:

		unexpected_token_syntax_err(next);

	end_arguments:

		uint ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(0); // fn_signature_idx, will be filled during type checking.
		ast.extra_data.push_back(arguments.size());
		ast.extra_data.insert(ast.extra_data.end(), arguments.begin(), arguments.end());

		uint function_call_node = ast.data.size();
		ast.tags.push_back(AstTag::FUNCTION_CALL);
		ast.data.push_back(NodeData {
			.function_call = {
				.callee_id = register_identifier(identifier_token.value),
				.ed_idx    = ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(identifier_token));
		return function_call_node;
	}

	/**
	 * Scans an offset expression.
	 * <identifier>[<expr>]
	 */
	uint
	scan_offset_expression(AST &ast, uint object_node)
	{
		// Consume the left bracket "[".

		Token left_bracket_token = next_token();
		assert_token_type(left_bracket_token, SPECIAL_CHARACTER);
		assert_token_value(left_bracket_token, "[");

		// Scan the expression.

		uint offset = scan_expression(ast);

		// Consume the right bracket "]".

		Token right_bracket_token = next_token();
		assert_token_type(right_bracket_token, SPECIAL_CHARACTER);
		assert_token_value(right_bracket_token, "]");

		uint offset_expr_node = ast.data.size();
		ast.tags.push_back(AstTag::OFFSET_EXPRESSION);
		ast.data.push_back(NodeData {
			.offset_expression = {
				.object_node = object_node,
				.offset_node = offset,
			},
		});
		ast.tokens.push_back(CompactToken(left_bracket_token));
		return offset_expr_node;
	}

	/**
	 * Scans a variable declaration.
	 * <type_id_pair> <identifier> (optional "= <expr>")
	 *
	 * @param type_id_pair The type identifier pair of the function.
	 * This was parsed by the `scan_declaration()` function.
	 */
	uint
	scan_variable_declaration(AST &ast, uint type_id_pair_node)
	{
		Token next = get_token();

		// TODO: add "," syntax to support multiple
		// declarations if needed.

		// Declaration + assignment

		if (next.type == OPERATOR && next.value == "=")
		{
			i++;
			uint assignment_expr_node = scan_expression(ast);

			uint id_expr_node = ast.data.size();
			ast.tags.push_back(AstTag::IDENTIFIER_EXPRESSION_STANDALONE);
			ast.data.push_back(NodeData {
				.identifier_expression_standalone = {
					.identifier_id = ast.data[type_id_pair_node].type_identifier_pair.identifier_id,
				},
			});
			ast.tokens.push_back(ast.tokens[type_id_pair_node]);

			uint ed_idx = ast.extra_data.size();
			ast.extra_data.push_back(id_expr_node);
			ast.extra_data.push_back(assignment_expr_node);

			uint node_idx = ast.data.size();
			ast.tags.push_back(AstTag::VARIABLE_DECLARATION_INITIALISED);
			ast.data.push_back(NodeData {
				.variable_declaration_initialised = {
					.type_and_id_node = type_id_pair_node,
					.ed_idx           = ed_idx,
				},
			});
			ast.tokens.push_back(ast.tokens[type_id_pair_node]);
			return node_idx;
		}

		// Only declaration

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::VARIABLE_DECLARATION_UNINITIALISED);
		ast.data.push_back(NodeData {
			.variable_declaration_uninitialised = {
				.type_and_id_node = type_id_pair_node,
			},
		});
		ast.tokens.push_back(ast.tokens[type_id_pair_node]);
		return node_idx;
	}

	/**
	 * Scans a class declaration.
	 * "class <class_name>" <code_block>
	 *
	 * The class body is parsed in the
	 * `ClassDeclaration` constructor.
	 */
	uint
	scan_class_declaration(AST &ast)
	{
		// Consume the "class" keyword.

		Token class_token = next_token();
		assert_token_type(class_token, KEYWORD);
		assert_token_value(class_token, "class");

		// Scan the class name.

		Token class_name_token = next_token();
		assert_token_type(class_name_token, TYPE);

		// Scan the class body.

		Token curly_brace_start = next_token();
		assert_token_type(curly_brace_start, SPECIAL_CHARACTER);
		assert_token_value(curly_brace_start, "{");

		std::vector<uint> fields;

		// Scan all fields.

		while (true)
		{
			Token maybe_curly_brace_end = get_token();

			if (maybe_curly_brace_end.type == SPECIAL_CHARACTER
				&& maybe_curly_brace_end.value == "}")
			{
				// End of class declaration, consume the right curly brace "}".

				i++;
				break;
			}

			fields.push_back(scan_type_identifier_pair(ast));
			expect_statement_terminator();
		}

		expect_statement_terminator();

		uint fields_ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(fields.size());
		ast.extra_data.insert(ast.extra_data.end(), fields.begin(), fields.end());

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::CLASS_DECLARATION);
		ast.data.push_back(NodeData {
			.class_declaration = {
				.class_id      = name_to_id[class_name_token.value],
				.fields_ed_idx = fields_ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(class_token));
		return node_idx;
	}

	/**
	 * Scans a return statement.
	 * "return (optional <expr>)
	 */
	uint
	scan_return_statement(AST &ast)
	{
		// Consume the "return" keyword.

		Token return_token = next_token();
		assert_token_type(return_token, KEYWORD);
		assert_token_value(return_token, "return");

		// Check if there is an expression.

		Token next = get_token();

		if (next.type == SPECIAL_CHARACTER && next.value == ";")
		{
			uint node_idx = ast.data.size();
			ast.tags.push_back(AstTag::RETURN_VOID_STATEMENT);
			ast.data.push_back(NodeData {});
			ast.tokens.push_back(CompactToken(return_token));
			return node_idx;
		}

		uint expression_node = scan_expression(ast);

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::RETURN_EXPRESSION_STATEMENT);
		ast.data.push_back(NodeData {
			.return_expression_statement = {
				.expression_node = expression_node,
			},
		});
		ast.tokens.push_back(CompactToken(return_token));
		return node_idx;
	}

	/**
	 * Scans an if-statement.
	 * "if (<expr>) <code_block or statement>"
	 */
	uint
	scan_if_statement(AST &ast)
	{
		// Consume the "if" keyword.

		Token if_token = next_token();
		assert_token_type(if_token, KEYWORD);
		assert_token_value(if_token, "if");

		// Consume the left parenthesis "(".

		Token left_parenthesis_token = next_token();
		assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(left_parenthesis_token, "(");

		// Scan the test expression.

		uint test = scan_expression(ast);

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan the then block.

		uint then_block = scan_code_block_or_statement(ast);

		// Check if there is an else or else if block.

		Token after_if_token = get_token();

		if (after_if_token.type == KEYWORD && after_if_token.value == "else")
		{
			i++;
			Token after_else_token = get_token();

			uint else_block;

			// Else if: wrap the block inside another
			// if block in the else block.
			// Example:
			// ```if (a) { .. } else if (b) { ... }```
			// Is turned into:
			// ```if (a) ... else { if (b) { ... } }```
			// This simplifies the process.

			if (after_else_token.type == KEYWORD
				&& after_else_token.value == "if")
			{
				uint nested_if_statement = scan_if_statement(ast);

				uint ed_idx = ast.extra_data.size();
				ast.extra_data.push_back(nested_if_statement);

				ast.tags.push_back(AstTag::CODE_BLOCK);
				ast.data.push_back(NodeData {
					.code_block = {
						.statements_len    = 1,
						.statements_ed_idx = ed_idx,
					},
				});
				ast.tokens.push_back(CompactToken(after_else_token));
			}

			// Normal else: simply scan the else block.

			else
			{
				else_block = scan_code_block_or_statement(ast);
			}

			uint ed_idx = ast.extra_data.size();
			ast.extra_data.push_back(then_block);
			ast.extra_data.push_back(else_block);

			uint node_idx = ast.data.size();
			ast.tags.push_back(AstTag::IF_ELSE_STATEMENT);
			ast.data.push_back(NodeData {
				.if_else_statement = {
					.condition_node = test,
					.ed_idx         = ed_idx,
				},
			});
			ast.tokens.push_back(CompactToken(if_token));
			return node_idx;
		}

		// There is no else block.

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::IF_STATEMENT);
		ast.data.push_back(NodeData {
			.if_statement = {
				.condition_node  = test,
				.then_block_node = then_block,
			},
		});
		ast.tokens.push_back(CompactToken(if_token));
		return node_idx;
	}

	/**
	 * Scans a syscall.
	 * "syscall <syscall_name>(<arguments>)"
	 */
	uint
	scan_syscall(AST &ast)
	{
		// Consume the "syscall" keyword.

		Token syscall_token = next_token();
		assert_token_type(syscall_token, KEYWORD);
		assert_token_value(syscall_token, "syscall");

		// Scan the syscall name.

		Token syscall_name_token = next_token();

		// Check if the syscall exists.

		SysCallId sys_call_id = str_to_sys_call_id(syscall_name_token.value);
		if (sys_call_id == SysCallId::UNKNOWN)
		{
			err_at_token(syscall_name_token, "Undefined System Call",
				"System call \"%s\" is not defined",
				syscall_name_token.value.c_str());
		}

		// Consume the left parenthesis "(".

		Token left_parethesis = next_token();
		assert_token_type(left_parethesis, SPECIAL_CHARACTER);
		assert_token_value(left_parethesis, "(");

		std::vector<uint> arguments;
		Token next = get_token();

		// Check if there are no arguments.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_arguments;
		}

		// Scan the arguments.

	next_argument:

		arguments.push_back(scan_expression(ast));
		next = next_token();

		if (next.type != SPECIAL_CHARACTER)
		{
			goto argument_err;
		}

		if (next.value == ")")
		{
			goto end_arguments;
		}

		if (next.value == ",")
		{
			goto next_argument;
		}

	argument_err:

		unexpected_token_syntax_err(next);

	end_arguments:

		expect_statement_terminator();

		uint ed_idx = ast.extra_data.size();
		ast.extra_data.insert(ast.extra_data.end(), arguments.begin(), arguments.end());

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::SYS_CALL);
		ast.data.push_back(NodeData {
			.sys_call = {
				.sys_call_id      = sys_call_id,
				.arguments_ed_idx = ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(syscall_token));
		return node_idx;
	}

	/**
	 * Scans a while statement.
	 * "while (<expr>) <code_block or statement>"
	 * TODO: maybe support python style while-else blocks?
	 */
	uint
	scan_while_statement(AST &ast)
	{
		// Consume the "while" keyword.

		Token while_token = next_token();
		assert_token_type(while_token, KEYWORD);
		assert_token_value(while_token, "while");

		// Consume the left parenthesis "(".

		Token left_parenthesis_token = next_token();
		assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(left_parenthesis_token, "(");

		// Scan the test expression.

		uint test = scan_expression(ast);

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan body block.

		uint body = scan_code_block_or_statement(ast);

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::WHILE_STATEMENT);
		ast.data.push_back(NodeData {
			.while_statement = {
				.condition_node = test,
				.body_node      = body,
			},
		});
		ast.tokens.push_back(CompactToken(while_token));
		return node_idx;
	}

	/**
	 * Scans a for statement.
	 * "for (<init> <test> <update>) <code_block or statement>"
	 */
	uint
	scan_for_statement(AST &ast)
	{
		// Consume the "for" keyword.

		Token for_token = next_token();
		assert_token_type(for_token, KEYWORD);
		assert_token_value(for_token, "for");

		// Consume the left parenthesis "(".

		Token left_parenthesis_token = next_token();
		assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(left_parenthesis_token, "(");

		// Scan the init statement.

		uint init = next_statement(ast);

		// Scan the test expression.

		uint test = scan_expression(ast);
		expect_statement_terminator();

		// Scan the update statement.

		uint update = scan_expression(ast);

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan the body block.

		uint body = scan_code_block_or_statement(ast);

		uint ed_idx = ast.extra_data.size();
		ast.extra_data.push_back(test);
		ast.extra_data.push_back(update);
		ast.extra_data.push_back(body);

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::FOR_STATEMENT);
		ast.data.push_back(NodeData {
			.for_statement = {
				.init_node   = init,
				.rest_ed_idx = ed_idx,
			},
		});
		ast.tokens.push_back(CompactToken(for_token));
		return node_idx;
	}

	/**
	 * Scans a break statement.
	 * "break"
	 */
	uint
	scan_break_statement(AST &ast)
	{
		Token break_token = next_token();
		assert_token_type(break_token, KEYWORD);
		assert_token_value(break_token, "break");

		expect_statement_terminator();

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::BREAK_STATEMENT);
		ast.data.push_back(NodeData {});
		ast.tokens.push_back(CompactToken(break_token));
		return node_idx;
	}

	/**
	 * Scans a continue statement.
	 */
	uint
	scan_continue_statement(AST &ast)
	{
		Token continue_token = next_token();
		assert_token_type(continue_token, KEYWORD);
		assert_token_value(continue_token, "continue");

		expect_statement_terminator();

		uint node_idx = ast.data.size();
		ast.tags.push_back(AstTag::CONTINUE_STATEMENT);
		ast.data.push_back(NodeData {});
		ast.tokens.push_back(CompactToken(continue_token));
		return node_idx;
	}
};

#endif