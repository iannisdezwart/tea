#ifndef TEA_PARSER_HEADER
#define TEA_PARSER_HEADER

#include <bits/stdc++.h>

#include "tokeniser.hpp"

// #define PARSER_VERBOSE

#include "ASTNodes/ASTNode.hpp"
#include "ASTNodes/TypeIdentifierPair.hpp"
#include "ASTNodes/VariableDeclaration.hpp"
#include "ASTNodes/FunctionDeclaration.hpp"
#include "ASTNodes/ReturnStatement.hpp"
#include "ASTNodes/CodeBlock.hpp"
#include "ASTNodes/LiteralStringExpression.hpp"
#include "ASTNodes/LiteralCharExpression.hpp"
#include "ASTNodes/LiteralNumberExpression.hpp"
#include "ASTNodes/IdentifierExpression.hpp"
#include "ASTNodes/FunctionCall.hpp"
#include "ASTNodes/BinaryOperation.hpp"
#include "ASTNodes/UnaryOperation.hpp"
#include "ASTNodes/AssignmentExpression.hpp"

using namespace std;

class Parser {
	private:
		vector<Token>& tokens;
		vector<ASTNode *> statements;
		size_t i = 0;

		#define syntax_err(token, message, ...) do { \
			fprintf(stderr, "[ Syntax Error ]: " message "\n", ##__VA_ARGS__); \
			fprintf(stderr, "At %ld:%ld\n", token.line, token.col); \
			abort(); \
		} while (0)

		#define unexpected_token_syntax_err(token) \
			syntax_err(token, "Unexpected token %s of type %d", \
				token.value.c_str(), token.type);

		#define assert_token_type(token, token_type) do { \
			if (token.type != token_type) { \
				fprintf(stderr, \
					"[ Syntax Error ]: Unexpected token of type %d, " \
					"expected a %d token.\n", \
					token.type, token_type); \
				fprintf(stderr, "At %ld:%ld\n", token.line, token.col); \
				abort(); \
			} \
		} while (0)

		#define assert_token_value(token, token_value) do { \
			if (token.value != token_value) { \
				fprintf(stderr, \
					"[ Syntax Error ]: Unexpected token with value \"%s\", " \
					"expected a token with value \"%s\".\n", \
					token.value.c_str(), token_value); \
				fprintf(stderr, "At %ld:%ld\n", token.line, token.col); \
				abort(); \
			} \
		} while (0)

	public:
		Parser(vector<Token>& tokens) : tokens(tokens) {}

		~Parser()
		{
			for (ASTNode *statement : statements) {
				statement->dfs([](ASTNode *node, size_t depth) {
					if (node != NULL) {
						#ifdef PARSER_VERBOSE
						printf("Deleted: %s\n", node->to_str().c_str());
						#endif

						// Weird hack that fixes memory leak

						if (node->type == CODE_BLOCK) {
							CodeBlock *cb = (CodeBlock *) node;
							cb->~CodeBlock();
						}

						delete node;
						node = NULL;
					}
				});
			}
		}

		vector<ASTNode *> parse()
		{
			while (i < tokens.size()) {
				ASTNode *statement = next_statement();

				statements.push_back(statement);
			}

			return statements;
		}

		void print_ast()
		{
			printf("\\\\\\ AST \\\\\\\n\n");

			for (ASTNode *statement : statements) {
				statement->dfs([](ASTNode *node, size_t depth) {
					for (size_t i = 0; i < depth; i++) putc('\t', stdout);
					node->print("\u279a");
				});
			}

			printf("\n/// AST ///\n");
		}

		Token get_token(size_t offset = 0)
		{
			if (i + offset >= tokens.size()) abort();
			return tokens[i + offset];
		}

		Token next_token()
		{
			if (i >= tokens.size()) abort();
			return tokens[i++];
		}

		ASTNode *next_statement()
		{
			Token token = get_token();
			ASTNode *node;

			switch (token.type) {
				case TYPE:
					return scan_declaration();

				case KEYWORD:
					if (token.value == "return") {
						node = scan_return_statement();
						expect_semicolon();
						return node;
					}
					// if (token.value == "if")
					// 	return scan_if_statement();
					// if (token.value == "loop")
					// 	return scan_loop_statement();
					// if (token.value == "while")
					// 	return scan_while_statement();
					// if (token.value == "break")
					// 	return scan_break_statement();
					// if (token.value == "for")
					// 	return scan_for_statement();
					// if (token.value == "continue")
					// 	return scan_continue_statement();
					// if (token.value == "goto")
					// 	return scan_goto_statement();
					else err("[ Parser Error ]: Keyword %s not handled.\n",
						token.value.c_str());

				case IDENTIFIER:
				{
					// Could be a function call or an assignment

					Token maybe_left_bracket = get_token(1);

					if (
						maybe_left_bracket.type == SPECIAL_CHARACTER &&
						maybe_left_bracket.value == "("
					) {
						node = scan_function_call();
						expect_semicolon();
						return node;
					} else {
						node = scan_expression();
						expect_semicolon();
						return node;
					}
				}

				// case OPERATOR:
				// 	// Prefix unary assignments

				// 	scan_prefix_unary_assignment();

				default:
					// unexpected_token_syntax_err(token);
					node = scan_expression();
					expect_semicolon();
					return node;
			}
		}

		void expect_semicolon()
		{
			Token semicolon = next_token();
			assert_token_type(semicolon, SPECIAL_CHARACTER);
			assert_token_value(semicolon, ";");
		}

		TypeIdentifierPair *scan_type_identifier_pair()
		{
			Token type_name_token = next_token();
			assert_token_type(type_name_token, TYPE);

			// Todo: add generics, &, * etc

			Token pointer_token = next_token();
			uint8_t pointer_depth = 0;

			while (pointer_token.type == OPERATOR) {
				for (size_t i = 0; i < pointer_token.value.size(); i++) {
					if (pointer_token.value[i] != '*') goto end_pointer;
				}

				pointer_depth += pointer_token.value.size();
				pointer_token = next_token();
			}

			end_pointer:

			Token& identifier_token = pointer_token;
			assert_token_type(identifier_token, IDENTIFIER);

			// Todo: add [] etc.

			return new TypeIdentifierPair(type_name_token, pointer_depth,
				identifier_token);
		}

		CodeBlock *scan_code_block()
		{
			Token curly_brace_start = next_token();
			assert_token_type(curly_brace_start, SPECIAL_CHARACTER);
			assert_token_value(curly_brace_start, "{");

			CodeBlock *code_block = new CodeBlock(curly_brace_start);

			while (true) {
				Token maybe_curly_brace_end = get_token();

				if (
					maybe_curly_brace_end.type == SPECIAL_CHARACTER
					&& maybe_curly_brace_end.value == "}"
				) {
					// End of code block

					i++;
					break;
				}

				code_block->add_statement(next_statement());
			}

			return code_block;
		}

		ASTNode *scan_expression()
		{
			vector<ASTNode *> expressions;
			vector<Token> operators;

			while (true) {
				expressions.push_back(scan_sub_expression());

				// Check if the next token is an operator

				Token maybe_operator_token = get_token();
				if (maybe_operator_token.type != OPERATOR) break;

				// It is an operator, push its token to the operator vector

				i++;
				operators.push_back(maybe_operator_token);
			}

			// Go through all existing operators, descending in precedence

			for (size_t i = 0; i < operator_precedence.size(); i++) {
				const OperatorPrecedencePair& o_p_pair = operator_precedence[i];
				const vector<Operator>& active_ops = o_p_pair.first;
				const Associativity& associativity = o_p_pair.second;

				// Go through the operators of the compound expression left-to-right

				if (associativity == LEFT_TO_RIGHT) {
					for (size_t j = 0; j < operators.size(); j++) {
						Token& op_token = operators[j];
						enum Operator op = str_to_operator(op_token.value);

						// Check if this operator is mergeable

						for (size_t k = 0; k < active_ops.size(); k++) {
							if (active_ops[k] == op) goto merge_op;
						}

						// It is not mergeable, check the next operator of the
						// compound expression

						continue;

						// It is mergeable into a BinaryOperation, so merge it

						merge_op:

						ASTNode *left = expressions[j];
						ASTNode *right = expressions[j + 1];
						ASTNode *new_expr;

						switch (op) {
							default:
								printf("Operator %d isn't yet implemented by the parser\n", op);
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
								new_expr = new BinaryOperation(left, right, op_token);
								break;
						}

						expressions[j] = new_expr;

						operators.erase(operators.begin() + j);
						expressions.erase(expressions.begin() + j + 1);

						j--;
					}
				}

				// Go through the operators of the compound expression right-to-left

				else {
					for (size_t j = operators.size(); j != 0; j--) {
						Token& op_token = operators[j - 1];
						enum Operator op = str_to_operator(op_token.value);

						// Check if this operator is mergeable

						for (size_t k = 0; k < active_ops.size(); k++) {
							if (active_ops[k] == op) goto merge_op_1;
						}

						// It is not mergeable, check the next operator of the
						// compound expression

						continue;

						// It is mergeable into a BinaryOperation, so merge it

						merge_op_1:

						ASTNode *left = expressions[j - 1];
						ASTNode *right = expressions[j];
						ASTNode *new_expr;

						switch (op) {
							default:
								printf("Operator %d isn't yet implemented by the parser\n", op);
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
								size_t dereference_depth = 0;
								Token id_token;

								while (left->type != IDENTIFIER_EXPRESSION) {
									if (left->type != UNARY_OPERATION)
										err_at_token(left->accountable_token,
											"Syntax Error",
											"Unexpected operator of type %d", op_token.type);

									UnaryOperation *unary_op = (UnaryOperation *) left;

									if (unary_op->op != DEREFERENCE)
										err_at_token(left->accountable_token,
											"Syntax Error",
											"Unexpected operator of type %d", op_token.type);

									left = unary_op->expression;
									delete unary_op;
									dereference_depth++;
								}

								IdentifierExpression *id_expr = (IdentifierExpression *) left;

								new_expr = new AssignmentExpression(
									id_expr->identifier_token, right, op_token, dereference_depth);

								delete id_expr;
								break;
							}
						}

						expressions[j - 1] = new_expr;

						operators.erase(operators.begin() + j - 1);
						expressions.erase(expressions.begin() + j);
					}
				}
			}

			return expressions[0];
		}

		ASTNode *scan_sub_expression()
		{
			Token first_token = get_token();

			// Parenthesised Expression
			// Todo: make casting work

			if (first_token.type == SPECIAL_CHARACTER && first_token.value == "(") {
				i++;

				ASTNode *expression = scan_expression();

				// Expect right parenthesis

				Token right_parenthesis = next_token();
				assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
				assert_token_value(right_parenthesis, ")");

				return expression;
			}

			vector<pair<Token, bool>> operators; // bool => true = prefix
			ASTNode *expression;

			// Prefix unary operators

			while (true) {
				// Check if the next token is an operator

				Token maybe_operator_token = get_token();
				if (maybe_operator_token.type != OPERATOR) break;

				enum Operator op = str_to_operator(maybe_operator_token.value, true);
				if (!is_prefix_unary_operator(op)) break;

				// It is a prefix unary operator, push its token to the operator vector

				printf("prefix unary op: %s\n", maybe_operator_token.to_str().c_str());
				i++;
				operators.push_back(make_pair(maybe_operator_token, true));
			}

			// if (first_token.type == OPERATOR) {
			// 	i++;

			// 	UnaryOperation *unary_operation =
			// 		new UnaryOperation(scan_expression(), first_token, true);

			// 	return unary_operation;
			// }

			Token expr_token = next_token();

			// Literal string

			if (expr_token.type == LITERAL_STRING) {
				expression = new LiteralStringExpression(expr_token, expr_token.value);

				// Todo: allow multiple literal strings next to each other
			}

			// Literal char

			else if (expr_token.type == LITERAL_CHAR) {
				expression = new LiteralCharExpression(expr_token);

				// Todo: make a method that parses a char correctly
			}

			// Literal number

			else if (expr_token.type == LITERAL_NUMBER) {
				expression = new LiteralNumberExpression(expr_token, expr_token.value);
			}

			// Identifier or FunctionCall

			else if (expr_token.type == IDENTIFIER) {
				Token next = get_token();

				// FunctionCall

				if (next.type == SPECIAL_CHARACTER && next.value == "(") {
					i--;
					return scan_function_call();
				}

				// IdentifierExpression

				expression = new IdentifierExpression(expr_token);
			}

			else unexpected_token_syntax_err(expr_token);

			// Postfix unary operators

			while (true) {
				// Check if the next token is an operator

				Token maybe_operator_token = get_token();
				if (maybe_operator_token.type != OPERATOR) break;

				enum Operator op = str_to_operator(maybe_operator_token.value);
				if (!is_postfix_unary_operator(op)) break;

				// It is a postfix unary operator, push its token to the operator vector

				printf("postfix unary op: %s\n", maybe_operator_token.to_str().c_str());
				i++;
				operators.push_back(make_pair(maybe_operator_token, false));
			}

			// Go through all existing operators, descending in precedence

			for (size_t i = 0; i < operator_precedence.size(); i++) {
				const OperatorPrecedencePair& o_p_pair = operator_precedence[i];
				const vector<Operator>& active_ops = o_p_pair.first;
				const Associativity& associativity = o_p_pair.second;

				// Go through the operators of the compound unary expression left-to-right

				if (associativity == LEFT_TO_RIGHT) {
					for (size_t j = 0; j < operators.size(); j++) {
						Token& op_token = operators[j].first;
						bool prefix = operators[j].second;
						enum Operator op = str_to_operator(op_token.value);

						// Check if this operator is mergeable

						for (size_t k = 0; k < active_ops.size(); k++) {
							if (active_ops[k] == op) goto merge_op;
						}

						// It is not mergeable, check the next operator of the
						// compound unary expression

						continue;

						// It is mergeable into a UnaryOperation, so merge it

						merge_op:

						ASTNode *new_expr = new UnaryOperation(expression, op_token, prefix);
						expression = new_expr;

						operators.erase(operators.begin() + j);

						j--;
					}
				}

				// Go through the operators of the compound unary expression right-to-left

				else {
					for (size_t j = operators.size(); j != 0; j--) {
						Token& op_token = operators[j - 1].first;
						bool prefix = operators[j - 1].second;
						enum Operator op = str_to_operator(op_token.value);

						// Check if this operator is mergeable

						for (size_t k = 0; k < active_ops.size(); k++) {
							if (active_ops[k] == op) goto merge_op_1;
						}

						// It is not mergeable, check the next operator of the
						// compound unary expression

						continue;

						// It is mergeable into a UnaryOperation, so merge it

						merge_op_1:

						ASTNode *new_expr = new UnaryOperation(expression, op_token, prefix);
						expression = new_expr;

						operators.erase(operators.begin() + j - 1);
					}
				}
			}

			return expression;
		}

		ASTNode *scan_declaration()
		{
			ASTNode *declaration;
			TypeIdentifierPair *type_id_pair = scan_type_identifier_pair();

			Token maybe_left_parenthesis = next_token();

			if (
				maybe_left_parenthesis.type == SPECIAL_CHARACTER
				&& maybe_left_parenthesis.value == "("
			) {
				// This is a function declaration

				declaration = scan_function_declaration(type_id_pair);
			} else {
				// This is a variable declaration

				i--;
				declaration = scan_variable_declaration(type_id_pair);

				expect_semicolon();
			}

			return declaration;
		}

		FunctionDeclaration *scan_function_declaration(
			TypeIdentifierPair *type_id_pair
		) {
			vector<TypeIdentifierPair *> params;
			Token next = get_token();

			// Check if there are no parameters

			if (next.type == SPECIAL_CHARACTER && next.value == ")") {
				i++;
				goto end_parameters;
			}

			// Scan parameters

			next_parameter:

			params.push_back(scan_type_identifier_pair());
			next = next_token();

			if (next.type != SPECIAL_CHARACTER) goto param_err;
			if (next.value == ")") goto end_parameters;
			if (next.value == ",") goto next_parameter;

			param_err:

			unexpected_token_syntax_err(next);

			end_parameters:

			CodeBlock *code_block = scan_code_block();

			return new FunctionDeclaration(type_id_pair, params, code_block);
		}

		FunctionCall *scan_function_call()
		{
			Token identifier_token = next_token();
			assert_token_type(identifier_token, IDENTIFIER);

			Token left_parenthesis_token = next_token();
			assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis_token, "(");

			vector<ASTNode *> arguments;
			Token next = get_token();

			// Check if there are no arguments

			if (next.type == SPECIAL_CHARACTER && next.value == ")") {
				i++;
				goto end_arguments;
			}

			// Scan parameters

			next_argument:

			arguments.push_back(scan_expression());
			next = next_token();

			if (next.type != SPECIAL_CHARACTER) goto argument_err;
			if (next.value == ")") goto end_arguments;
			if (next.value == ",") goto next_argument;

			argument_err:

			unexpected_token_syntax_err(next);

			end_arguments:

			return new FunctionCall(identifier_token, arguments);
		}

		VariableDeclaration *scan_variable_declaration(
			TypeIdentifierPair *type_id_pair
		) {
			Token next = next_token();

			// Todo: add "," syntax
			// Only declaration

			if (next.type == SPECIAL_CHARACTER && next.value == ";") {
				i--;
				return new VariableDeclaration(type_id_pair, NULL);
			}

			// Declaration + assignment

			assert_token_type(next, OPERATOR);
			assert_token_value(next, "=");

			return new VariableDeclaration(type_id_pair, scan_expression());
		}

		ReturnStatement *scan_return_statement()
		{
			Token return_token = next_token();
			assert_token_type(return_token, KEYWORD);
			assert_token_value(return_token, "return");

			Token next = get_token();

			if (next.type == SPECIAL_CHARACTER && next.value == ";") {
				return new ReturnStatement(return_token, NULL);
			}

			ASTNode *expression = scan_expression();
			ReturnStatement *return_statement = new ReturnStatement(
				return_token, expression);

			return return_statement;
		}
};

#endif