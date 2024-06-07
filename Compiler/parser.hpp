#ifndef TEA_PARSER_HEADER
#define TEA_PARSER_HEADER

#include <vector>
#include "Compiler/tokeniser.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
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
merge_bin_ops_ltr(std::vector<std::unique_ptr<ReadValue>> &expressions,
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

		std::unique_ptr<ReadValue> &left_expr  = expressions[j];
		std::unique_ptr<ReadValue> &right_expr = expressions[j + 1];
		std::unique_ptr<ReadValue> new_expr;

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case MULTIPLICATION:
		case DIVISION:
		case REMAINDER:
		case ADDITION:
		case SUBTRACTION:
		case BITWISE_AND:
		case BITWISE_XOR:
		case BITWISE_OR:
		case LEFT_SHIFT:
		case RIGHT_SHIFT:
		case LESS:
		case LESS_OR_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case EQUAL:
		case NOT_EQUAL:
		{
			new_expr = std::make_unique<BinaryOperation>(CompactToken(op_token),
				str_to_operator(op_token.value),
				std::move(left_expr), std::move(right_expr));
			break;
		}

		case POINTER_TO_MEMBER:
		case DEREFERENCED_POINTER_TO_MEMBER:
		{
			if (right_expr->node_type == IDENTIFIER_EXPRESSION)
			{
				std::unique_ptr<WriteValue> object =
					static_unique_ptr_cast<WriteValue>(std::move(left_expr));
				std::unique_ptr<IdentifierExpression> member =
					static_unique_ptr_cast<IdentifierExpression>(std::move(right_expr));

				new_expr = std::make_unique<MemberExpression>(CompactToken(op_token),
					str_to_operator(op_token.value),
					std::move(object), std::move(member));
				break;
			}

			err_at_token(right_expr->accountable_token, "Type Error",
				"A member of a class instance must be an IdentifierExpression "
				"or a FunctionCall\n"
				"Found a %s",
				ast_node_type_to_str(right_expr->node_type));
		} // case DEREFERENCED_POINTER_TO_MEMBER
		} // switch

		expressions[j] = std::move(new_expr);

		operators.erase(operators.begin() + j);
		expressions.erase(expressions.begin() + j + 1);

		j--;
	}
}

/**
 * @brief Merges binary operators with right-to-left associativity.
 */
void
merge_bin_ops_rtl(std::vector<std::unique_ptr<ReadValue>> &expressions,
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

		std::unique_ptr<ReadValue> &left  = expressions[j - 1];
		std::unique_ptr<ReadValue> &right = expressions[j];
		std::unique_ptr<ReadValue> new_expr;

		switch (op)
		{
		default:
			p_warn(stderr, "Operator %s isn't yet implemented by the parser\n",
				op_to_str(op));
			abort();

		case ASSIGNMENT:
		case SUM_ASSIGNMENT:
		case DIFFERENCE_ASSIGNMENT:
		case QUOTIENT_ASSIGNMENT:
		case REMAINDER_ASSIGNMENT:
		case PRODUCT_ASSIGNMENT:
		case LEFT_SHIFT_ASSIGNMENT:
		case RIGHT_SHIFT_ASSIGNMENT:
		case BITWISE_AND_ASSIGNMENT:
		case BITWISE_XOR_ASSIGNMENT:
		case BITWISE_OR_ASSIGNMENT:
		{
			// size_t dereference_depth = 0;
			// Token id_token;

			// while (left->type != IDENTIFIER_EXPRESSION) {
			// 	if (left->type != UNARY_OPERATION)
			// 		err_at_token(left->accountable_token,
			// 			"Syntax Error",
			// 			"Unexpected operator of type %d", op_token.type);

			// 	UnaryOperation *unary_op = (UnaryOperation *) left;

			// 	if (unary_op->op != DEREFERENCE)
			// 		err_at_token(left->accountable_token,
			// 			"Syntax Error",
			// 			"Unexpected operator of type %d", op_token.type);

			// 	left = unary_op->expression;
			// 	delete unary_op;
			// 	dereference_depth++;
			// }

			new_expr = std::make_unique<AssignmentExpression>(CompactToken(op_token),
				str_to_operator(op_token.value),
				std::unique_ptr<WriteValue>(WriteValue::cast(left.release())),
				std::move(right));

			break;
		} // case BITWISE_OR_ASSIGNMENT
		} // switch

		expressions[j - 1] = std::move(new_expr);

		operators.erase(operators.begin() + j - 1);
		expressions.erase(expressions.begin() + j);
	}
}

/**
 * Merges unary operators with left-to-right associativity.
 */
void
merge_un_ops_ltr(std::unique_ptr<ReadValue> &expression,
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

		std::unique_ptr<ReadValue> new_expr = std::make_unique<UnaryOperation>(
			CompactToken(op_token), str_to_operator(op_token.value, prefix),
			prefix, std::move(expression));
		expression = std::move(new_expr);

		operators.erase(operators.begin() + j);

		j--;
	}
}

/**
 * @brief Merges unary operators with right-to-left associativity.
 */
void
merge_un_ops_rtl(std::unique_ptr<ReadValue> &expression,
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

		std::unique_ptr<ReadValue> new_expr = std::make_unique<UnaryOperation>(
			CompactToken(op_token), str_to_operator(op_token.value, prefix),
			prefix, std::move(expression));

		expression = std::move(new_expr);

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

	// Mapping of class names to IDs.
	std::unordered_map<std::string, uint32_t> class_name_to_id;
	std::unordered_map<uint32_t, std::string> class_id_to_name;

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
	std::tuple<std::vector<std::unique_ptr<ASTNode>>, std::unordered_map<uint32_t, std::string>>
	parse()
	{
		std::vector<std::unique_ptr<ASTNode>> statements;

		scan_class_names();

		while (i < tokens.size())
		{
			statements.push_back(next_statement());
		}

		return std::make_tuple(std::move(statements), std::move(class_id_to_name));
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

				uint32_t class_id                        = class_name_to_id.size() + BUILTIN_TYPE_END;
				class_name_to_id[class_name_token.value] = class_id;
				class_id_to_name[class_id]               = class_name_token.value;
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

		// User defined class

		if (token.type == IDENTIFIER && class_name_to_id.count(token.value))
		{
			token.type = TYPE;
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

	/**
	 * @brief Looks at the next couple of tokens and figures out
	 * what kind of statement they represent. The statement
	 * is then parsed and returned as an AST node.
	 */
	std::unique_ptr<ASTNode>
	next_statement()
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

			return scan_declaration();

		case KEYWORD:
			if (token.value == "return")
			{
				std::unique_ptr<ASTNode> node = scan_return_statement();
				expect_statement_terminator();
				return node;
			}

			if (token.value == "if")
			{
				return scan_if_statement();
			}

			if (token.value == "while")
			{
				return scan_while_statement();
			}

			if (token.value == "for")
			{
				return scan_for_statement();
			}

			if (token.value == "class")
			{
				return scan_class_declaration();
			}

			if (token.value == "syscall")
			{
				return scan_syscall();
			}

			if (token.value == "break")
			{
				return scan_break_statement();
			}

			if (token.value == "continue")
			{
				return scan_continue_statement();
			}

			else
				err("[ Parser Error ]: Keyword %s not handled.\n",
					token.value.c_str());

		default:
		{
			std::unique_ptr<ASTNode> node = scan_expression();
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
	std::unique_ptr<TypeName>
	scan_type_name()
	{
		Token type_name_token = next_token();
		assert_token_type(type_name_token, TYPE);

		// TODO: add generics and references maybe

		std::vector<size_t> array_sizes;
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
				array_sizes.push_back(stoull(size_token.value));

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

		if (class_name_to_id.count(type_name_token.value))
		{
			return std::make_unique<TypeName>(CompactToken(type_name_token),
				type_name_token.value,
				std::make_optional(class_name_to_id[type_name_token.value]),
				std::move(array_sizes));
		}

		return std::make_unique<TypeName>(CompactToken(type_name_token),
			type_name_token.value, std::nullopt, std::move(array_sizes));
	}

	/**
	 * Scans a pair of a type and an identifier.
	 * <type> <identifier_name>
	 *
	 * Useful when scanning a declaration.
	 */
	std::unique_ptr<TypeIdentifierPair>
	scan_type_identifier_pair()
	{
		std::unique_ptr<TypeName> type_name = scan_type_name();

		Token identifier_token = next_token();
		assert_token_type(identifier_token, IDENTIFIER);

		return std::make_unique<TypeIdentifierPair>(
			CompactToken(identifier_token), std::move(type_name),
			identifier_token.value);
	}

	/**
	 * Scans a code block. Note that in the Tea language
	 * a single statement is also allowed instead of a code block.
	 * This is handled by the `scan_code_block_or_statement()`
	 * method.
	 * "{ any number of <statement> }"
	 */
	std::unique_ptr<CodeBlock>
	scan_code_block()
	{
		// Expect a left curly brace "{".

		Token curly_brace_start = next_token();
		assert_token_type(curly_brace_start, SPECIAL_CHARACTER);
		assert_token_value(curly_brace_start, "{");

		std::unique_ptr<CodeBlock> code_block = std::make_unique<CodeBlock>(
			CompactToken(curly_brace_start));

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

			code_block->add_statement(next_statement());
		}

		return code_block;
	}

	/**
	 * Scans a single statement and wraps it inside a code block.
	 * Used by the `scan_code_block_or_statement()` method,
	 * to support single line statements instead of code blocks.
	 * <statement>
	 */
	std::unique_ptr<CodeBlock>
	scan_and_wrap_statement_inside_code_block()
	{
		// Scan the statement.

		Token first_token                  = get_token();
		std::unique_ptr<ASTNode> statement = next_statement();

		// Wrap it inside a code block.

		std::unique_ptr<CodeBlock> code_block = std::make_unique<CodeBlock>(
			CompactToken(first_token));
		code_block->add_statement(std::move(statement));

		return code_block;
	}

	/**
	 * Scans either a code block, or a single statement.
	 * <code_block> or <statement>
	 *
	 * Used to scan a statement or a code block that comes
	 * after an if, while, for etc.
	 */
	std::unique_ptr<CodeBlock>
	scan_code_block_or_statement()
	{
		Token first_token = get_token();

		// Check if there is an opening curly brace

		if (first_token.type == SPECIAL_CHARACTER && first_token.value == "{")
		{
			// If so, scan the code block

			return scan_code_block();
		}
		else
		{
			// Else, scan the statement and wrap it inside a code block

			return scan_and_wrap_statement_inside_code_block();
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
	std::unique_ptr<ReadValue>
	scan_expression()
	{
		// List of the expressions
		std::vector<std::unique_ptr<ReadValue>> expressions;

		// List of all operators between the expressions
		std::vector<Token> operators;

		// Scans all sub expressions and operators that
		// connect them.
		// This loop stops when the next token is not
		// an operator.

		while (true)
		{
			expressions.push_back(scan_sub_expression());

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
				merge_bin_ops_ltr(expressions, operators, active_ops);
			}
			else
			{
				merge_bin_ops_rtl(expressions, operators, active_ops);
			}
		}

		std::unique_ptr<ReadValue> expression = std::move(expressions[0]);

		// OffsetExpression

		for (Token next = get_token();
			next.type == SPECIAL_CHARACTER && next.value == "[";
			next = get_token())
		{
			expression = scan_offset_expression(std::unique_ptr<WriteValue>(
				WriteValue::cast(expression.release())));
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
	std::unique_ptr<ReadValue>
	scan_sub_expression()
	{
		std::vector<std::pair<Token, bool>> operators; // bool => true = prefix
		std::unique_ptr<ReadValue> expression;

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

			std::unique_ptr<TypeName> type_name;

			if (class_name_to_id.count(expr_token.value))
			{
				type_name = std::make_unique<TypeName>(
					CompactToken(expr_token), expr_token.value,
					std::make_optional(class_name_to_id[expr_token.value]),
					std::vector<size_t> {});
			}
			else
			{
				type_name = std::make_unique<TypeName>(
					CompactToken(expr_token), expr_token.value, std::nullopt,
					std::vector<size_t> {});
			}

			expression = std::make_unique<CastExpression>(
				std::move(type_name), scan_expression());

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
			std::unique_ptr<TypeName> type_name = scan_type_name();

			Token right_square_bracket = next_token();
			assert_token_type(right_square_bracket, SPECIAL_CHARACTER);
			assert_token_value(right_square_bracket, "]");

			Token left_parenthesis = next_token();
			assert_token_type(left_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis, "(");

			expression = std::make_unique<CastExpression>(
				std::move(type_name), scan_expression());

			Token right_parenthesis = next_token();
			assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis, ")");
		}

		// Parenthesised Expression
		// (<expr>)

		else if (expr_token.type == SPECIAL_CHARACTER && expr_token.value == "(")
		{
			expression = scan_expression();

			// Expect right parenthesis

			Token right_parenthesis = next_token();
			assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis, ")");
		}

		// Literal string

		else if (expr_token.type == LITERAL_STRING)
		{
			expression = std::make_unique<LiteralStringExpression>(
				CompactToken(expr_token), expr_token.value);

			// TODO: allow multiple literal strings next to each other.
		}

		// Literal char

		else if (expr_token.type == LITERAL_CHAR)
		{
			expression = std::make_unique<LiteralCharExpression>(
				CompactToken(expr_token), expr_token.value[0]);

			// TODO: make a method that parses a char correctly.
			// Currently, it will just parse the first char.
		}

		// Literal number

		else if (expr_token.type == LITERAL_NUMBER)
		{
			expression = std::make_unique<LiteralNumberExpression>(
				CompactToken(expr_token), expr_token.value);
		}

		// IdentifierExpression or FunctionCall

		else if (expr_token.type == IDENTIFIER)
		{
			Token next = get_token();

			// FunctionCall

			if (next.type == SPECIAL_CHARACTER && next.value == "(")
			{
				i--;
				expression = scan_function_call();
			}

			// IdentifierExpression

			else
			{
				expression = std::make_unique<IdentifierExpression>(
					CompactToken(expr_token), expr_token.value);
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
				merge_un_ops_ltr(expression, operators, active_ops);
			}
			else
			{
				merge_un_ops_rtl(expression, operators, active_ops);
			}
		}

		return expression;
	}

	/**
	 * Scans a declaration. This can either be a function
	 * declaration or a variable declaration.
	 */
	std::unique_ptr<ASTNode>
	scan_declaration()
	{
		std::unique_ptr<TypeIdentifierPair> type_id_pair = scan_type_identifier_pair();

		Token maybe_left_parenthesis = next_token();

		if (maybe_left_parenthesis.type == SPECIAL_CHARACTER
			&& maybe_left_parenthesis.value == "(")
		{
			// This is a function declaration

			return scan_function_declaration(std::move(type_id_pair));
		}

		// This is a variable declaration

		i--;
		std::unique_ptr<VariableDeclaration> var_decl =
			scan_variable_declaration(std::move(type_id_pair));
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
	std::unique_ptr<FunctionDeclaration>
	scan_function_declaration(
		std::unique_ptr<TypeIdentifierPair> type_id_pair)
	{
		std::vector<std::unique_ptr<TypeIdentifierPair>> params;
		Token next = get_token();

		// Check if there are no parameters.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_parameters;
		}

		// Scan parameters.

	next_parameter:

		params.push_back(scan_type_identifier_pair());
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

		std::unique_ptr<CodeBlock> code_block = scan_code_block();

		return std::make_unique<FunctionDeclaration>(
			std::move(type_id_pair), std::move(params), std::move(code_block));
	}

	/**
	 * Scans a function body.
	 * <identifier>(<parameter_list>)
	 */
	std::unique_ptr<FunctionCall>
	scan_function_call()
	{
		// Scan the function name.

		Token identifier_token = next_token();
		assert_token_type(identifier_token, IDENTIFIER);

		// Consume the left paranthesis "(".

		Token left_parenthesis_token = next_token();
		assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(left_parenthesis_token, "(");

		std::vector<std::unique_ptr<ReadValue>> arguments;
		Token next = get_token();

		// Check if there are no arguments.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_arguments;
		}

		// Scan the arguments.

	next_argument:

		arguments.push_back(scan_expression());
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

		return std::make_unique<FunctionCall>(CompactToken(identifier_token),
			identifier_token.value, std::move(arguments));
	}

	/**
	 * Scans an offset expression.
	 * <identifier>[<expr>]
	 */
	std::unique_ptr<OffsetExpression>
	scan_offset_expression(std::unique_ptr<WriteValue> left)
	{
		// Consume the left bracket "[".

		Token left_bracket_token = next_token();
		assert_token_type(left_bracket_token, SPECIAL_CHARACTER);
		assert_token_value(left_bracket_token, "[");

		// Scan the expression.

		std::unique_ptr<ReadValue> offset = scan_expression();

		// Consume the right bracket "]".

		Token right_bracket_token = next_token();
		assert_token_type(right_bracket_token, SPECIAL_CHARACTER);
		assert_token_value(right_bracket_token, "]");

		return std::make_unique<OffsetExpression>(CompactToken(left_bracket_token),
			std::move(left), std::move(offset));
	}

	/**
	 * Scans a variable declaration.
	 * <type_id_pair> <identifier> (optional "= <expr>")
	 *
	 * @param type_id_pair The type identifier pair of the function.
	 * This was parsed by the `scan_declaration()` function.
	 */
	std::unique_ptr<VariableDeclaration>
	scan_variable_declaration(std::unique_ptr<TypeIdentifierPair> type_id_pair)
	{
		Token next = get_token();

		// TODO: add "," syntax to support multiple
		// declarations if needed.

		// Declaration + assignment

		if (next.type == OPERATOR && next.value == "=")
		{
			i++;
			return std::make_unique<VariableDeclaration>(
				std::move(type_id_pair), scan_expression());
		}

		// Only declaration

		return std::make_unique<VariableDeclaration>(
			std::move(type_id_pair), nullptr);
	}

	/**
	 * Scans a class declaration.
	 * "class <class_name>" <code_block>
	 *
	 * The class body is parsed in the
	 * `ClassDeclaration` constructor.
	 */
	std::unique_ptr<ClassDeclaration>
	scan_class_declaration()
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

		std::vector<std::unique_ptr<TypeIdentifierPair>> fields;

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

			fields.push_back(scan_type_identifier_pair());
			expect_statement_terminator();
		}

		expect_statement_terminator();

		return std::make_unique<ClassDeclaration>(CompactToken(class_token),
			class_name_to_id[class_name_token.value], std::move(fields));
	}

	/**
	 * Scans a return statement.
	 * "return (optional <expr>)
	 */
	std::unique_ptr<ReturnStatement>
	scan_return_statement()
	{
		// Consume the "return" keyword.

		Token return_token = next_token();
		assert_token_type(return_token, KEYWORD);
		assert_token_value(return_token, "return");

		// Check if there is an expression.

		Token next = get_token();

		if (next.type == SPECIAL_CHARACTER && next.value == ";")
		{
			return std::make_unique<ReturnStatement>(
				CompactToken(return_token), nullptr);
		}

		// TODO: check if the expression is empty and there
		// is no semicolon. I think it does not work yet.

		std::unique_ptr<ReturnStatement> return_statement =
			std::make_unique<ReturnStatement>(
				CompactToken(return_token), scan_expression());

		return return_statement;
	}

	/**
	 * Scans an if-statement.
	 * "if (<expr>) <code_block or statement>"
	 */
	std::unique_ptr<IfStatement>
	scan_if_statement()
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

		std::unique_ptr<ReadValue> test = scan_expression();

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan the then block.

		std::unique_ptr<CodeBlock> then_block = scan_code_block_or_statement();

		// Check if there is an else or else if block.

		Token after_if_token = get_token();

		if (after_if_token.type == KEYWORD && after_if_token.value == "else")
		{
			i++;
			Token after_else_token = get_token();

			std::unique_ptr<CodeBlock> else_block;

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
				std::unique_ptr<IfStatement> nested_if_statement = scan_if_statement();

				else_block = std::make_unique<CodeBlock>(
					CompactToken(after_else_token));
				else_block->add_statement(std::move(nested_if_statement));
			}

			// Normal else: simply scan the else block.

			else
			{
				else_block = scan_code_block_or_statement();
			}

			return std::make_unique<IfStatement>(CompactToken(if_token),
				std::move(test), std::move(then_block), std::move(else_block));
		}

		// There is no else block.

		return std::make_unique<IfStatement>(CompactToken(if_token),
			std::move(test), std::move(then_block), nullptr);
	}

	/**
	 * Scans a syscall.
	 * "syscall <syscall_name>(<arguments>)"
	 */
	std::unique_ptr<SysCall>
	scan_syscall()
	{
		// Consume the "syscall" keyword.

		Token syscall_token = next_token();
		assert_token_type(syscall_token, KEYWORD);
		assert_token_value(syscall_token, "syscall");

		// Scan the syscall name.

		Token syscall_name_token = next_token();

		// Check if the syscall exists.

		if (!syscall_names.count(syscall_name_token.value))
		{
			err_at_token(syscall_name_token, "Undefined System Call",
				"System call \"%s\" is not defined",
				syscall_name_token.value.c_str());
		}

		// Consume the left parenthesis "(".

		Token left_parethesis = next_token();
		assert_token_type(left_parethesis, SPECIAL_CHARACTER);
		assert_token_value(left_parethesis, "(");

		std::vector<std::unique_ptr<ReadValue>> arguments;
		Token next = get_token();

		// Check if there are no arguments.

		if (next.type == SPECIAL_CHARACTER && next.value == ")")
		{
			i++;
			goto end_arguments;
		}

		// Scan the arguments.

	next_argument:

		arguments.push_back(scan_expression());
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
		return std::make_unique<SysCall>(CompactToken(syscall_name_token),
			syscall_name_token.value, std::move(arguments));
	}

	/**
	 * Scans a while statement.
	 * "while (<expr>) <code_block or statement>"
	 * TODO: maybe support python style while-else blocks?
	 */
	std::unique_ptr<WhileStatement>
	scan_while_statement()
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

		std::unique_ptr<ReadValue> test = scan_expression();

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan body block.

		std::unique_ptr<CodeBlock> body = scan_code_block_or_statement();

		return std::make_unique<WhileStatement>(CompactToken(while_token),
			std::move(test), std::move(body));
	}

	/**
	 * Scans a for statement.
	 * "for (<init> <test> <update>) <code_block or statement>"
	 */
	std::unique_ptr<ForStatement>
	scan_for_statement()
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

		std::unique_ptr<ASTNode> init = next_statement();

		// Scan the test expression.

		std::unique_ptr<ReadValue> test = scan_expression();
		expect_statement_terminator();

		// Scan the update statement.

		std::unique_ptr<ReadValue> update = scan_expression();

		// Consume the right parenthesis ")".

		Token right_parenthesis_token = next_token();
		assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
		assert_token_value(right_parenthesis_token, ")");

		// Scan the body block.

		std::unique_ptr<CodeBlock> body = scan_code_block_or_statement();

		return std::make_unique<ForStatement>(CompactToken(for_token),
			std::move(init), std::move(test), std::move(update), std::move(body));
	}

	/**
	 * Scans a break statement.
	 * "break"
	 */
	std::unique_ptr<BreakStatement>
	scan_break_statement()
	{
		Token break_token = next_token();
		assert_token_type(break_token, KEYWORD);
		assert_token_value(break_token, "break");

		expect_statement_terminator();
		return std::make_unique<BreakStatement>(CompactToken(break_token));
	}

	/**
	 * Scans a continue statement.
	 */
	std::unique_ptr<ContinueStatement>
	scan_continue_statement()
	{
		Token continue_token = next_token();
		assert_token_type(continue_token, KEYWORD);
		assert_token_value(continue_token, "continue");

		expect_statement_terminator();
		return std::make_unique<ContinueStatement>(CompactToken(continue_token));
	}
};

#endif