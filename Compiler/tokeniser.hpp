#ifndef TEA_TOKENISER_HEADER
#define TEA_TOKENISER_HEADER

#include <bits/stdc++.h>

#include "util.hpp"

using namespace std;

enum TokenType {
	TYPE,
	LITERAL_STRING,
	LITERAL_CHAR,
	LITERAL_NUMBER,
	IDENTIFIER,
	KEYWORD,
	OPERATOR,
	SPECIAL_CHARACTER
};

struct Token {
	TokenType type;
	string value;
	size_t line;
	size_t col;
};

unordered_set<char> whitespace_chars = {
	' ', '\t', '\n', '\r'
};

unordered_set<char> special_chars = {
	'(', ')', '{', '}', '[', ']', ',', '.', ';', '"', '\'', '\\'
};

unordered_set<char> operator_chars = {
	'+', '-', '*', '/', '%', '=', '<', '>', '&', '|', '!', '~'
};

unordered_set<string> types = {
	"int", "string", "void"
};

unordered_set<string> keywords = {
	"if", "else", "return", "loop", "while", "for", "break", "continue", "goto"
};

class Tokeniser {
	private:
		FILE *file;
		size_t line = 1;
		size_t col = 1;

		char get_char()
		{
			char c = fgetc(file);

			if (c == '\n') {
				line++;
				col = 0;
			} else {
				col++;
			}

			return c;
		}

		void unget_char(char c)
		{
			ungetc(c, file);

			if (col == 0) {
				printf("Undefined behaviour incoming...\n");
				line--;
			} else {
				col--;
			}
		}

		void push_token(TokenType type, string value)
		{
			Token token = {
				.type = type,
				.value = value,
				.line = line,
				.col = col
			};

			tokens.push_back(token);
		}

		#define throw_err(message, ...) do { \
			fprintf(stderr, "[ Tokenise Error ]: " message "\n", ##__VA_ARGS__); \
			fprintf(stderr, "At %ld:%ld\n", line, col); \
			exit(1); \
		} while (0)

	public:
		vector<Token> tokens;

		Tokeniser(FILE *input_file) : file(input_file) {}

		void print_tokens()
		{
			printf("\\\\\\ Tokens (%ld) \\\\\\\n\n", tokens.size());

			for (Token token : tokens) {
				printf("{ type: %d, value: \"%s\", line: %ld, col: %ld }\n",
					token.type, token.value.c_str(), token.line, token.col);
			}

			printf("\n/// Tokens ///\n");
		}

		vector<Token> tokenise()
		{
			size_t line = 1;
			size_t col = 1;
			char c;

			while (true) {
				c = get_char();

				if (c == EOF) break;
				if (whitespace_chars.count(c)) continue;

				// Handle comments

				if (c == '/') {
					if (scan_comment()) {
						continue;
					}
				}

				// Handle special character

				if (special_chars.count(c)) {
					push_token(SPECIAL_CHARACTER, string(1, c));
				}

				// Handle operator

				else if (operator_chars.count(c)) {
					push_token(OPERATOR, string(1, c));
				}

				// Handle string literals

				else if (c == '"') {
					push_token(LITERAL_STRING, scan_literal_string());
				}

				// Handle character literals

				else if (c == '\'') {
					push_token(LITERAL_CHAR, scan_literal_char());
				}

				// Handle number literals

				else if (c >= '0' && c <= '9' || c == '.') {
					push_token(LITERAL_NUMBER, scan_literal_number(c));
				}

				// Handle text (keywords and symbols)

				else if (is_alpha(c)) {
					string text = scan_text(c);

					// If the token is a type

					if (types.count(text))
						push_token(TYPE, text);

					// If the token is a keyword

					else if (keywords.count(text))
						push_token(KEYWORD, text);

					// It's probably an identifier

					else push_token(IDENTIFIER, text);
				}

				// Unknown character

				else throw_err("Found unknown character '%c'", c);
			}

			return tokens;
		}

		bool scan_comment()
		{
			char next_char = get_char();

			if (next_char == '/') {
				// Ignore all characters until the next '\n'

				while (next_char != '\n' && next_char != EOF)
					next_char = get_char();

				return true;
			} else {
				unget_char(next_char);
				return false;
			}
		}

		string scan_literal_string()
		{
			string s;
			char c;

			while (true) {
				c = get_char();
				if (c == EOF) throw_err("Unexpected EOF");

				// Todo: fix escape sequences

				// Stop on end of string

				if (c == '"') {
					break;
				}

				// Add this character to the string

				s += c;
			}

			return s;
		}

		string scan_literal_char()
		{
			char c = get_char();
			if (c == EOF) throw_err("Unexpected EOF");

			// Todo: fix escape sequences

			char next = get_char();
			if (next == EOF) throw_err("Unexpected EOF");
			if (next != '\'') throw_err("Unexpected char '%c'", next);

			return string(1, c);
		}

		string scan_literal_number(char first_char)
		{
			string s;
			s += first_char;
			char c;

			if (first_char == '0') {
				// We could have to deal with 0x or 0b prefix, or a decimal seperator

				c = get_char();

				if (c == 'x' || c == 'b' || c == '.') {
					do {
						s += c;
						c = get_char();
					} while (c >= '0' && c <= '9');

					unget_char(c);
					return s;
				}

				if (c < '0' && c > '9' && c != '.')
					throw_err("Unexpected char '%c'", c);
			}

			if (c >= '0' && c <= '9' || c == '.') {
				do {
					s += c;
					c = get_char();
				} while (c >= '0' && c <= '9' || c == '.');
			}

			unget_char(c);
			return s;
		}

		string scan_text(char first_char)
		{
			string s;
			s += first_char;
			char c;

			while (true) {
				c = get_char();

				if (!is_alphanumeric(c)) {
					unget_char(c);
					break;
				}

				s += c;
			}

			return s;
		}
};

#endif