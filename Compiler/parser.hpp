#ifndef TEA_PARSER_HEADER
#define TEA_PARSER_HEADER

#include <bits/stdc++.h>

#include "tokeniser.hpp"

#include "ASTNodes/ASTNode.hpp"
#include "ASTNodes/WriteValue.hpp"
#include "ASTNodes/ReadValue.hpp"
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
#include "ASTNodes/MemberExpression.hpp"
#include "ASTNodes/MethodCall.hpp"
#include "ASTNodes/AssignmentExpression.hpp"
#include "ASTNodes/IfStatement.hpp"
#include "ASTNodes/WhileStatement.hpp"
#include "ASTNodes/ClassDeclaration.hpp"
#include "ASTNodes/InitList.hpp"

using namespace std;

class Parser {
	private:
		vector<Token>& tokens;
		vector<ASTNode *> statements;
		size_t i = 0;
		unordered_set<string> class_names;

		#define syntax_err(token, message, ...) do { \
			fprintf(stderr, "[ Syntax Error ]: " message "\n", ##__VA_ARGS__); \
			fprintf(stderr, "At %ld:%ld\n", token.line, token.col); \
			abort(); \
		} while (0)

		#define unexpected_token_syntax_err(token) \
			syntax_err(token, "Unexpected token %s of type %s", \
				token.value.c_str(), token_type_to_str(token.type));

		#define assert_token_type(token, token_type) do { \
			if (token.type != token_type) { \
				fprintf(stderr, \
					"[ Syntax Error ]: Unexpected token of type %s, " \
					"expected a %s token.\n", \
					token_type_to_str(token.type), token_type_to_str(token_type)); \
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
			Token token = tokens[i + offset];

			// User defined class

			if (token.type == IDENTIFIER && class_names.count(token.value)) {
				token.type = TYPE;
			}

			return token;
		}

		Token next_token()
		{
			Token token = get_token();
			i++;
			return token;
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
						expect_statement_terminator();
						return node;
					}

					if (token.value == "if") {
						return scan_if_statement();
					}

					if (token.value == "while") {
						return scan_while_statement();
					}

					if (token.value == "class") {
						return scan_class_declaration();
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

				// case IDENTIFIER:
				// {
				// 	// Could be a function call or an assignment

				// 	Token maybe_left_bracket = get_token(1);

				// 	if (
				// 		maybe_left_bracket.type == SPECIAL_CHARACTER &&
				// 		maybe_left_bracket.value == "("
				// 	) {
				// 		node = scan_function_call();
				// 		expect_statement_terminator();
				// 		return node;
				// 	} else {
				// 		node = scan_expression();
				// 		expect_statement_terminator();
				// 		return node;
				// 	}
				// }

				// case OPERATOR:
				// 	// Prefix unary assignments

				// 	scan_prefix_unary_assignment();

				default:
					// unexpected_token_syntax_err(token);
					node = scan_expression();
					expect_statement_terminator();
					return node;
			}
		}

		void expect_statement_terminator()
		{
			Token terminator = get_token();

			if (terminator.type == SPECIAL_CHARACTER && terminator.value == ";") {
				i++;
				return;
			}

			if (terminator.whitespace_before) return;

			fprintf(stderr,
				"[ Syntax Error ]: Unexpected token with value \"%s\", "
				"expected a terminator token (newline or semicolon).\n",
				terminator.value.c_str());
			fprintf(stderr, "At %ld:%ld\n", terminator.line, terminator.col);
			abort();
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

		CodeBlock *scan_and_wrap_statement_inside_code_block()
		{
			Token first_token = get_token();
			ASTNode *statement = next_statement();
			CodeBlock *code_block = new CodeBlock(first_token);
			code_block->add_statement(statement);
			return code_block;
		}

		CodeBlock *scan_code_block_or_statement()
		{
			Token first_token = get_token();

			// Check if there is an opening curly brace

			if (first_token.type == SPECIAL_CHARACTER && first_token.value == "{") {
				// If so, scan the code block

				return scan_code_block();
			} else {
				// Else, scan the statement and wrap it inside a code block

				return scan_and_wrap_statement_inside_code_block();
			}
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

						ASTNode *left_expr = expressions[j];
						ASTNode *right_expr = expressions[j + 1];
						ASTNode *new_expr;

						switch (op) {
							default:
								printf("Operator %s isn't yet implemented by the parser\n",
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
								new_expr = new BinaryOperation(ReadValue::cast(left_expr),
									ReadValue::cast(right_expr), op_token);
								break;
							}

							case POINTER_TO_MEMBER:
							case DEREFERENCED_POINTER_TO_MEMBER:
							{
								if (left_expr->type != IDENTIFIER_EXPRESSION) {
									err_at_token(left_expr->accountable_token, "Type Error",
										"Cannot use pointer to member operator on a non-identifier");
								}

								if (right_expr->type == IDENTIFIER_EXPRESSION) {
									IdentifierExpression *object = (IdentifierExpression *) left_expr;
									IdentifierExpression *member = (IdentifierExpression *) right_expr;

									new_expr = new MemberExpression(object, member, op_token);
									break;
								}

								if (right_expr->type == FUNCTION_CALL) {
									IdentifierExpression *object = (IdentifierExpression *) left_expr;
									FunctionCall *method = (FunctionCall *) right_expr;

									new_expr = new MethodCall(object, method, op_token);
									break;
								}

								err_at_token(right_expr->accountable_token, "Type Error",
									"A member of a class instance must be an IdentifierExpression "
									"or a FunctionCall\n"
									"Found a %s", ast_node_type_to_str(right_expr->type));
							}
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
								printf("Operator %s isn't yet implemented by the parser\n",
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

								new_expr = new AssignmentExpression(WriteValue::cast(left),
									ReadValue::cast(right), op_token);

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

				i++;
				operators.push_back(make_pair(maybe_operator_token, true));
			}

			Token expr_token = next_token();

			// Parenthesised Expression
			// Todo: make casting work

			if (expr_token.type == SPECIAL_CHARACTER && expr_token.value == "(") {
				expression = scan_expression();

				// Expect right parenthesis

				Token right_parenthesis = next_token();
				assert_token_type(right_parenthesis, SPECIAL_CHARACTER);
				assert_token_value(right_parenthesis, ")");
			}

			// Init list

			else if (expr_token.type == SPECIAL_CHARACTER && expr_token.value == "{") {
				vector<ReadValue *> items;
				Token maybe_end_token = get_token();
				Token seperator;

				if (maybe_end_token.type == SPECIAL_CHARACTER && maybe_end_token.value == "}") {
					i++;
					goto end_init_list;
				}

				next_init_list_item:
				items.push_back(ReadValue::cast(scan_expression()));

				seperator = next_token();

				if (seperator.type != SPECIAL_CHARACTER || seperator.value != "}"
					&& seperator.value != ",")
				{
					err_at_token(seperator, "Syntax Error",
						"Unexpected token \"%s\" of type %s\n"
						"Expected a \",\" or a \"}\" token instead\n"
						"At %lu:%lu\n", seperator.value.c_str(),
						token_type_to_str(seperator.type), seperator.line, seperator.col);
				}

				if (seperator.value == ",") {
					goto next_init_list_item;
				}

				end_init_list:
				expression = new InitList(expr_token, std::move(items));
			}

			// Literal string

			else if (expr_token.type == LITERAL_STRING) {
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
					expression = scan_function_call();
				}

				// IdentifierExpression

				else {
					expression = new IdentifierExpression(expr_token);
				}
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

						ASTNode *new_expr = new UnaryOperation(ReadValue::cast(expression),
							op_token, prefix);

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

						ASTNode *new_expr = new UnaryOperation(ReadValue::cast(expression),
							op_token, prefix);

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

				expect_statement_terminator();
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

			vector<ReadValue *> arguments;
			Token next = get_token();

			// Check if there are no arguments

			if (next.type == SPECIAL_CHARACTER && next.value == ")") {
				i++;
				goto end_arguments;
			}

			// Scan parameters

			next_argument:

			arguments.push_back(ReadValue::cast(scan_expression()));
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
			Token next = get_token();

			// Todo: add "," syntax
			// Declaration + assignment

			if (next.type == OPERATOR && next.value == "=") {
				i++;
				return new VariableDeclaration(type_id_pair, ReadValue::cast(scan_expression()));
			}

			// Only declaration

			expect_statement_terminator();
			return new VariableDeclaration(type_id_pair, NULL);
		}

		ClassDeclaration *scan_class_declaration()
		{
			Token class_token = next_token();
			assert_token_type(class_token, KEYWORD);
			assert_token_value(class_token, "class");

			Token class_name_token = next_token();
			assert_token_type(class_name_token, IDENTIFIER);
			class_names.insert(class_name_token.value);

			CodeBlock *body = scan_code_block();
			return new ClassDeclaration(class_token, class_name_token.value, body);
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

			ReturnStatement *return_statement = new ReturnStatement(
				return_token, ReadValue::cast(scan_expression()));

			return return_statement;
		}

		IfStatement *scan_if_statement()
		{
			Token if_token = next_token();
			assert_token_type(if_token, KEYWORD);
			assert_token_value(if_token, "if");

			// Scan test

			Token left_parenthesis_token = next_token();
			assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis_token, "(");

			ASTNode *test = scan_expression();

			Token right_parenthesis_token = next_token();
			assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis_token, ")");

			// Scan then block

			CodeBlock *then_block = scan_code_block_or_statement();

			// See what comes after the then block

			Token after_if_token = get_token();

			if (after_if_token.type == KEYWORD && after_if_token.value == "else") {
				i++;
				Token after_else_token = get_token();

				CodeBlock *else_block;

				// Else if

				if (
					after_else_token.type == KEYWORD
					&& after_else_token.value == "if"
				) {
					else_block = new CodeBlock(after_else_token);
					IfStatement *nested_if_statement = scan_if_statement();
					else_block->add_statement(nested_if_statement);
				}

				// Normal else

				else {
					else_block = scan_code_block_or_statement();
				}

				return new IfStatement(test, if_token, then_block, else_block);
			}

			return new IfStatement(test, if_token, then_block, NULL);
		}

		WhileStatement *scan_while_statement()
		{
			Token while_token = next_token();
			assert_token_type(while_token, KEYWORD);
			assert_token_value(while_token, "while");

			// Scan test

			Token left_parenthesis_token = next_token();
			assert_token_type(left_parenthesis_token, SPECIAL_CHARACTER);
			assert_token_value(left_parenthesis_token, "(");

			ASTNode *test = scan_expression();

			Token right_parenthesis_token = next_token();
			assert_token_type(right_parenthesis_token, SPECIAL_CHARACTER);
			assert_token_value(right_parenthesis_token, ")");

			// Scan body block

			CodeBlock *body = scan_code_block_or_statement();

			return new WhileStatement(test, while_token, body);
		}
};

#endif