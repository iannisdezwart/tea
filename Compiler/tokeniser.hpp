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

	string to_str()
	{
		string s = "Token { type = " + to_string(type) + ", value = \"" + value +
			"\" , line = " + to_string(line) + " , col = " + to_string(col)
			+ " } @ " + to_hex(this);
		return s;
	}
};

unordered_set<char> whitespace_chars = {
	' ', '\t', '\n', '\r'
};

unordered_set<char> special_chars = {
	'(', ')', '{', '}', '[', ']', ',', '.', ';', '"', '\'', '\\'
};

unordered_set<char> operator_chars = {
	'+', '-', '*', '/', '%', '=', '<', '>', '&', '^', '|', '!', '~'
};

unordered_set<string> types = {
	"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64", "void"
};

unordered_set<string> keywords = {
	"if", "else", "return", "loop", "while", "for", "break", "continue", "goto"
};

enum Operator {
	POSTFIX_INCREMENT,
	POSTFIX_DECREMENT,

	PREFIX_INCREMENT,
	PREFIX_DECREMENT,

	UNARY_PLUS,
	UNARY_MINUS,

	BITWISE_NOT,
	LOGICAL_NOT,

	DEREFERENCE,
	ADDRESS_OF,

	POINTER_TO_MEMBER,
	DEREFERENCED_POINTER_TO_MEMBER,

	DIVISION,
	REMAINDER,
	MULTIPLICATION,

	ADDITION,
	SUBTRACTION,

	LEFT_SHIFT,
	RIGHT_SHIFT,

	LESS,
	LESS_OR_EQUAL,
	GREATER,
	GREATER_OR_EQUAL,

	EQUAL,
	NOT_EQUAL,

	BITWISE_AND,
	BITWISE_XOR,
	BITWISE_OR,

	LOGICAL_AND,
	LOGICAL_OR,

	ASSIGNMENT,

	SUM_ASSIGNMENT,
	DIFFERENCE_ASSIGNMENT,

	QUOTIENT_ASSIGNMENT,
	REMAINDER_ASSIGNMENT,
	PRODUCT_ASSIGNMENT,

	LEFT_SHIFT_ASSIGNMENT,
	RIGHT_SHIFT_ASSIGNMENT,

	BITWISE_AND_ASSIGNMENT,
	BITWISE_XOR_ASSIGNMENT,
	BITWISE_OR_ASSIGNMENT,

	UNDEFINED
};

const char *op_to_str(enum Operator op)
{
	switch (op) {
		case POSTFIX_INCREMENT: return "POSTFIX_INCREMENT";
		case POSTFIX_DECREMENT: return "POSTFIX_DECREMENT";
		case PREFIX_INCREMENT: return "PREFIX_INCREMENT";
		case PREFIX_DECREMENT: return "PREFIX_DECREMENT";
		case UNARY_PLUS: return "UNARY_PLUS";
		case UNARY_MINUS: return "UNARY_MINUS";
		case BITWISE_NOT: return "BITWISE_NOT";
		case LOGICAL_NOT: return "LOGICAL_NOT";
		case DEREFERENCE: return "DEREFERENCE";
		case ADDRESS_OF: return "ADDRESS_OF";
		case POINTER_TO_MEMBER: return "POINTER_TO_MEMBER";
		case DEREFERENCED_POINTER_TO_MEMBER: return "DEREFERENCED_POINTER_TO_MEMBER";
		case DIVISION: return "DIVISION";
		case REMAINDER: return "REMAINDER";
		case MULTIPLICATION: return "MULTIPLICATION";
		case ADDITION: return "ADDITION";
		case SUBTRACTION: return "SUBTRACTION";
		case LEFT_SHIFT: return "LEFT_SHIFT";
		case RIGHT_SHIFT: return "RIGHT_SHIFT";
		case LESS: return "LESS";
		case LESS_OR_EQUAL: return "LESS_OR_EQUAL";
		case GREATER: return "GREATER";
		case GREATER_OR_EQUAL: return "GREATER_OR_EQUAL";
		case EQUAL: return "EQUAL";
		case NOT_EQUAL: return "NOT_EQUAL";
		case BITWISE_AND: return "BITWISE_AND";
		case BITWISE_XOR: return "BITWISE_XOR";
		case BITWISE_OR: return "BITWISE_OR";
		case LOGICAL_AND: return "LOGICAL_AND";
		case LOGICAL_OR: return "LOGICAL_OR";
		case ASSIGNMENT: return "ASSIGNMENT";
		case SUM_ASSIGNMENT: return "SUM_ASSIGNMENT";
		case DIFFERENCE_ASSIGNMENT: return "DIFFERENCE_ASSIGNMENT";
		case QUOTIENT_ASSIGNMENT: return "QUOTIENT_ASSIGNMENT";
		case REMAINDER_ASSIGNMENT: return "REMAINDER_ASSIGNMENT";
		case PRODUCT_ASSIGNMENT: return "PRODUCT_ASSIGNMENT";
		case LEFT_SHIFT_ASSIGNMENT: return "LEFT_SHIFT_ASSIGNMENT";
		case RIGHT_SHIFT_ASSIGNMENT: return "RIGHT_SHIFT_ASSIGNMENT";
		case BITWISE_AND_ASSIGNMENT: return "BITWISE_AND_ASSIGNMENT";
		case BITWISE_XOR_ASSIGNMENT: return "BITWISE_XOR_ASSIGNMENT";
		case BITWISE_OR_ASSIGNMENT: return "BITWISE_OR_ASSIGNMENT";
		default: return "UNDEFINED";
	}
}

const char *op_to_example_str(enum Operator op)
{
	switch (op) {
		case POSTFIX_INCREMENT: return "x++";
		case POSTFIX_DECREMENT: return "x--";
		case PREFIX_INCREMENT: return "++x";
		case PREFIX_DECREMENT: return "--x";
		case UNARY_PLUS: return "+x";
		case UNARY_MINUS: return "-x";
		case BITWISE_NOT: return "~x";
		case LOGICAL_NOT: return "!x";
		case DEREFERENCE: return "*x";
		case ADDRESS_OF: return "&x";
		case POINTER_TO_MEMBER: return "x.y";
		case DEREFERENCED_POINTER_TO_MEMBER: return "x->y";
		case DIVISION: return "x / y";
		case REMAINDER: return "x % y";
		case MULTIPLICATION: return "x * y";
		case ADDITION: return "x + y";
		case SUBTRACTION: return "x - y";
		case LEFT_SHIFT: return "x << y";
		case RIGHT_SHIFT: return "x >> y";
		case LESS: return "x < y";
		case LESS_OR_EQUAL: return "x <= y";
		case GREATER: return "x > y";
		case GREATER_OR_EQUAL: return "x >= y";
		case EQUAL: return "x == y";
		case NOT_EQUAL: return "x != y";
		case BITWISE_AND: return "x & y";
		case BITWISE_XOR: return "x ^ y";
		case BITWISE_OR: return "x | y";
		case LOGICAL_AND: return "x && y";
		case LOGICAL_OR: return "x || y";
		case ASSIGNMENT: return "x = y";
		case SUM_ASSIGNMENT: return "x += y";
		case DIFFERENCE_ASSIGNMENT: return "x -= y";
		case QUOTIENT_ASSIGNMENT: return "x /= y";
		case REMAINDER_ASSIGNMENT: return "x &= y";
		case PRODUCT_ASSIGNMENT: return "x *= y";
		case LEFT_SHIFT_ASSIGNMENT: return "x <<= y";
		case RIGHT_SHIFT_ASSIGNMENT: return "x >>= y";
		case BITWISE_AND_ASSIGNMENT: return "x &= y";
		case BITWISE_XOR_ASSIGNMENT: return "x ^= y";
		case BITWISE_OR_ASSIGNMENT: return "x |= y";
		default: return "undefined";
	}
}

bool is_prefix_unary_operator(enum Operator op)
{
	switch (op) {
		case PREFIX_INCREMENT:
		case PREFIX_DECREMENT:
		case UNARY_PLUS:
		case UNARY_MINUS:
		case BITWISE_NOT:
		case LOGICAL_NOT:
		case DEREFERENCE:
		case ADDRESS_OF:
			return true;

		default:
			return false;
	}
}

bool is_postfix_unary_operator(enum Operator op)
{
	switch (op) {
		case POSTFIX_INCREMENT:
		case POSTFIX_DECREMENT:
			return true;

		default:
			return false;
	}
}

bool is_binary_operator(enum Operator op)
{
	switch (op) {
		case POINTER_TO_MEMBER:
		case DEREFERENCED_POINTER_TO_MEMBER:
		case DIVISION:
		case REMAINDER:
		case MULTIPLICATION:
		case ADDITION:
		case SUBTRACTION:
		case LEFT_SHIFT:
		case RIGHT_SHIFT:
		case LESS:
		case LESS_OR_EQUAL:
		case GREATER:
		case GREATER_OR_EQUAL:
		case EQUAL:
		case NOT_EQUAL:
		case BITWISE_AND:
		case BITWISE_XOR:
		case BITWISE_OR:
		case LOGICAL_AND:
		case LOGICAL_OR:
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
			return true;

		default:
			return false;
	}
}

enum Associativity {
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT
};

typedef pair<vector<Operator>, enum Associativity> OperatorPrecedencePair;
#define mp make_pair<vector<Operator>, enum Associativity>

vector<OperatorPrecedencePair> operator_precedence = {
	mp({ POSTFIX_INCREMENT, POSTFIX_DECREMENT }, LEFT_TO_RIGHT),
	mp({ PREFIX_INCREMENT, PREFIX_DECREMENT, UNARY_PLUS, UNARY_MINUS,
		BITWISE_NOT, LOGICAL_NOT, DEREFERENCE, ADDRESS_OF }, RIGHT_TO_LEFT),
	mp({ POINTER_TO_MEMBER, DEREFERENCED_POINTER_TO_MEMBER }, LEFT_TO_RIGHT),
	mp({ MULTIPLICATION, DIVISION, REMAINDER }, LEFT_TO_RIGHT),
	mp({ ADDITION, SUBTRACTION }, LEFT_TO_RIGHT),
	mp({ LEFT_SHIFT, RIGHT_SHIFT }, LEFT_TO_RIGHT),
	mp({ LESS, LESS_OR_EQUAL, GREATER, GREATER_OR_EQUAL }, LEFT_TO_RIGHT),
	mp({ EQUAL, NOT_EQUAL }, LEFT_TO_RIGHT),
	mp({ BITWISE_AND }, LEFT_TO_RIGHT),
	mp({ BITWISE_XOR }, LEFT_TO_RIGHT),
	mp({ BITWISE_OR }, LEFT_TO_RIGHT),
	mp({ LOGICAL_AND }, LEFT_TO_RIGHT),
	mp({ LOGICAL_OR }, LEFT_TO_RIGHT),
	mp({ ASSIGNMENT, SUM_ASSIGNMENT, DIFFERENCE_ASSIGNMENT, QUOTIENT_ASSIGNMENT,
		REMAINDER_ASSIGNMENT, PRODUCT_ASSIGNMENT, LEFT_SHIFT_ASSIGNMENT,
		RIGHT_SHIFT_ASSIGNMENT, BITWISE_AND_ASSIGNMENT, BITWISE_XOR_ASSIGNMENT,
		BITWISE_OR_ASSIGNMENT }, RIGHT_TO_LEFT)
};

#undef mp

enum Operator str_to_operator(string& str, bool prefix = false)
{
	if (str.size() > 3) return UNDEFINED;

	if (str == "++" && prefix) return PREFIX_INCREMENT;
	if (str == "--" && prefix) return PREFIX_DECREMENT;
	if (str == "++") return POSTFIX_INCREMENT;
	if (str == "--") return POSTFIX_DECREMENT;
	if (str == "+" && prefix) return UNARY_PLUS;
	if (str == "-" && prefix) return UNARY_MINUS;
	if (str == "~" && prefix) return BITWISE_NOT;
	if (str == "!" && prefix) return LOGICAL_NOT;
	if (str == "*" && prefix) return DEREFERENCE;
	if (str == "&" && prefix) return ADDRESS_OF;
	if (str == ".") return POINTER_TO_MEMBER;
	if (str == "->") return DEREFERENCED_POINTER_TO_MEMBER;
	if (str == "*") return MULTIPLICATION;
	if (str == "/") return DIVISION;
	if (str == "%") return REMAINDER;
	if (str == "+") return ADDITION;
	if (str == "-") return SUBTRACTION;
	if (str == "<<") return LEFT_SHIFT;
	if (str == ">>") return RIGHT_SHIFT;
	if (str == "<") return LESS;
	if (str == "<=") return LESS_OR_EQUAL;
	if (str == ">") return GREATER;
	if (str == ">=") return GREATER_OR_EQUAL;
	if (str == "==") return EQUAL;
	if (str == "!=") return NOT_EQUAL;
	if (str == "&") return BITWISE_AND;
	if (str == "^") return BITWISE_XOR;
	if (str == "|") return BITWISE_OR;
	if (str == "&&") return LOGICAL_AND;
	if (str == "||") return LOGICAL_OR;
	if (str == "=") return ASSIGNMENT;
	if (str == "+=") return SUM_ASSIGNMENT;
	if (str == "-=") return DIFFERENCE_ASSIGNMENT;
	if (str == "/=") return QUOTIENT_ASSIGNMENT;
	if (str == "%=") return REMAINDER_ASSIGNMENT;
	if (str == "*=") return PRODUCT_ASSIGNMENT;
	if (str == "<<=") return LEFT_SHIFT_ASSIGNMENT;
	if (str == ">>=") return RIGHT_SHIFT_ASSIGNMENT;
	if (str == "&=") return BITWISE_AND_ASSIGNMENT;
	if (str == "^=") return BITWISE_XOR_ASSIGNMENT;
	if (str == "|=") return BITWISE_OR_ASSIGNMENT;
	return UNDEFINED;
}

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
			abort(); \
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

				if (c == '#') {
					ignore_until_eol();
					continue;
				}

				// Handle string literals

				if (c == '"') {
					push_token(LITERAL_STRING, scan_literal_string());
				}

				// Handle character literals

				else if (c == '\'') {
					push_token(LITERAL_CHAR, string(1, scan_literal_char()));
				}

				// Handle special character

				else if (special_chars.count(c)) {
					push_token(SPECIAL_CHARACTER, string(1, c));
				}

				// Handle operator

				else if (operator_chars.count(c)) {
					push_token(OPERATOR, scan_operator(c));
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

				else throw_err("Found unknown character '%c' (%hhd)", c, c);
			}

			return tokens;
		}

		bool scan_comment()
		{
			char next_char = get_char();

			if (next_char == '/') {
				ignore_until_eol();

				return true;
			} else {
				unget_char(next_char);
				return false;
			}
		}

		void ignore_until_eol()
		{
			char next_char = get_char();

			while (next_char != '\n' && next_char != EOF)
				next_char = get_char();
		}

		char scan_escape_sequence()
		{
			char c = get_char();

			switch (c) {
				case 'a':
					return '\x07';

				case 'b':
					return '\x08';

				case 'e':
					return '\x01b';

				case 'f':
					return '\x0c';

				case 'n':
					return '\x0a';

				case 'r':
					return '\x0d';

				case 't':
					return '\x09';

				case 'v':
					return '\x0b';

				case '\\':
					return '\x5c';

				case '\'':
					return '\x27';

				case '"':
					return '\x22';

				case '?':
					return '\x3f';

				case 'x':
				{
					char xc1 = get_char();
					char xc2 = get_char();

					if (!is_hex(xc1) || !is_hex(xc2))
						throw_err("Invalid hexadecimal escape code");

					return hex_chars_to_byte(xc1, xc2);
				}

				default:
					throw_err("Invalid escape code");
			}
		}

		string scan_literal_string()
		{
			string s;
			char c;

			while (true) {
				c = get_char();
				if (c == EOF) throw_err("Unexpected EOF");

				// Escape sequence

				if (c == '\\') {
					s += scan_escape_sequence();
					continue;
				}

				// Stop on end of string

				if (c == '"') {
					break;
				}

				// Add this character to the string

				s += c;
			}

			return s;
		}

		char scan_literal_char()
		{
			char c = get_char();
			if (c == EOF) throw_err("Unexpected EOF");

			// Escape sequence

			if (c == '\\')
				c = scan_escape_sequence();

			char next = get_char();
			if (next == EOF) throw_err("Unexpected EOF");
			if (next != '\'')
				throw_err("Unexpected char '%c', expected ending quote", next);

			return c;
		}

		string scan_literal_number(char first_char)
		{
			string s;
			s += first_char;
			char c = get_char();

			if (first_char == '0') {
				// We could have to deal with 0x or 0b prefix, or a decimal seperator

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

		string scan_operator(char first_char)
		{
			// Check if the next two characters are maybe also operators

			char op_char_2 = get_char();

			if (!operator_chars.count(op_char_2)) {
				// Only the first character is an operator

				unget_char(op_char_2);
			} else {
				// The second character is an operator
				// Check if the third is too

				char op_char_3 = get_char();

				if (!operator_chars.count(op_char_3)) {
					// Only the first two characters are operators

					unget_char(op_char_3);
				} else {
					// The third character is also an operator
					// Check if we can form a triple character operator

					string op;
					op += first_char;
					op += op_char_2;
					op += op_char_3;

					if (str_to_operator(op) != UNDEFINED) return op;
					else unget_char(op_char_3);
				}

				// Check if we can form a double character operator

				string op;
				op += first_char;
				op += op_char_2;

				if (str_to_operator(op) != UNDEFINED) return op;
				else unget_char(op_char_2);
			}

			string op;
			op += first_char;
			return op;

			// op += first_char;
			// op += first_char;
			// char c;

			// while (true) {
			// 	c = get_char();

			// 	if (!operator_chars.count(c)) {
			// 		unget_char(c);
			// 		break;
			// 	}

			// 	op += c;
			// }

			// // if (str_to_operator(op) == UNDEFINED)
			// // 	throw_err("Found unknown operator combination \"%s\"", op.c_str());

			// return op;
		}
};

#endif