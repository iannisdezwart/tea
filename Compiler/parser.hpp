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
						// node = scan_assignment();
						expect_semicolon();
						printf("assignment not implemented yet\n");
						abort();
						return node;
					}
				}

				// case OPERATOR:
				// 	// Prefix unary assignments

				// 	scan_prefix_unary_assignment();

				default:
					unexpected_token_syntax_err(token);
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

			Token identifier_token = next_token();
			assert_token_type(identifier_token, IDENTIFIER);

			// Todo: add [] etc.

			return new TypeIdentifierPair(type_name_token, identifier_token);
		}

		CodeBlock *scan_code_block()
		{
			Token curly_brace_start = next_token();
			assert_token_type(curly_brace_start, SPECIAL_CHARACTER);
			assert_token_value(curly_brace_start, "{");

			CodeBlock *code_block = new CodeBlock();

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

			// Literal string

			if (first_token.type == LITERAL_STRING) {
				i++;

				LiteralStringExpression *expression =
					new LiteralStringExpression(first_token.value);

				// Todo: allow multiple literal strings next to each other

				return expression;
			}

			// Literal char

			if (first_token.type == LITERAL_CHAR) {
				i++;

				LiteralCharExpression *expression =
					new LiteralCharExpression(0);

				// Todo: make a method that parses a char correctly

				return expression;
			}

			// Literal number

			if (first_token.type == LITERAL_NUMBER) {
				i++;

				LiteralNumberExpression *expression =
					new LiteralNumberExpression(first_token.value);

				Token next = get_token();

				if (next.type == OPERATOR) {
					// Todo: create
				}

				return expression;
			}

			// Identifier or FunctionCall

			if (first_token.type == IDENTIFIER) {
				Token next = get_token(1);

				// FunctionCall

				printf("first_token.type was an IDENTIFIER\n");
				printf("%s\n", next.to_str().c_str());
				if (next.type == SPECIAL_CHARACTER && next.value == "(") {
					return scan_function_call();
				}

				// IdentifierExpression

				i++;

				IdentifierExpression *expression =
					new IdentifierExpression(first_token);

				if (next.type == OPERATOR) {
					// Todo: create
				}

				if (next.type == SPECIAL_CHARACTER) {
					// Todo: do something with:
					// - "[" (member access)
					// - "." (member access)
					// - "(" (function call)
				}

				return expression;
			}

			unexpected_token_syntax_err(first_token);
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
			Token next = next_token();
			vector<TypeIdentifierPair *> params;

			// Scan parameters

			while (!(next.type == SPECIAL_CHARACTER && next.value == ")")) {
				params.push_back(scan_type_identifier_pair());
				next = next_token();

				if (next.type != SPECIAL_CHARACTER)
					unexpected_token_syntax_err(next);

				if (next.value == ",") continue;
				if (next.value == ")") break;

				unexpected_token_syntax_err(next);
			}

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
			Token next = next_token();

			// Scan arguments

			while (!(next.type == SPECIAL_CHARACTER && next.value == ")")) {
				arguments.push_back(scan_expression());
				next = next_token();

				if (next.type != SPECIAL_CHARACTER)
					unexpected_token_syntax_err(next);

				if (next.value == ",") continue;
				if (next.value == ")") break;

				unexpected_token_syntax_err(next);
			}

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
				return new ReturnStatement(NULL);
			}

			ASTNode *expression = scan_expression();
			ReturnStatement *return_statement = new ReturnStatement(expression);

			return return_statement;
		}
};

#endif