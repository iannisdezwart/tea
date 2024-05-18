#ifndef TEA_TOKENISER_HEADER
#define TEA_TOKENISER_HEADER

#include <vector>
#include <unordered_set>

#include "Compiler/file-stream-reader.hpp"
#include "Compiler/util.hpp"
#include "Shared/ansi.hpp"

/**
 * @brief Enum containing all the possible types of tokens.
 */
enum TokenType
{
	TYPE,
	LITERAL_STRING,
	LITERAL_CHAR,
	LITERAL_NUMBER,
	IDENTIFIER,
	KEYWORD,
	OPERATOR,
	SPECIAL_CHARACTER
};

/**
 * @brief Converts a token type to a string.
 * @param type The token type.
 * @returns A string representation of the token type.
 */
const char *
token_type_to_str(TokenType type)
{
	switch (type)
	{
	case TYPE:
		return "TYPE";
	case LITERAL_STRING:
		return "LITERAL_STRING";
	case LITERAL_CHAR:
		return "LITERAL_CHAR";
	case LITERAL_NUMBER:
		return "LITERAL_NUMBER";
	case IDENTIFIER:
		return "IDENTIFIER";
	case KEYWORD:
		return "KEYWORD";
	case OPERATOR:
		return "OPERATOR";
	case SPECIAL_CHARACTER:
		return "SPECIAL_CHARACTER";
	default:
		return "UNDEFINED";
	}
}

/**
 * @param b The boolean to be converted to a string.
 * @returns A boolean as a string.
 */
const char *
bool_to_string(bool b)
{
	if (b)
		return "true";
	else
		return "false";
}

/**
 * @brief Structure for a token.
 * Holds the type and value of a token.
 * Also holds the line and column of the token.
 * TODO: add the file name of the token when supporting imports.
 */
struct Token
{
	// The type of the token.
	TokenType type;

	// The value of the token as a string.
	std::string value;

	// The line in the source file where the token was found.
	size_t line;

	// The column in the source file where the token was found.
	size_t col;

	// Boolean indicating whether the token has a whitespace before it.
	// This is used to determine line termination.
	bool whitespace_before;

	/**
	 * @returns A string representation of the token.
	 */
	std::string
	to_str() const
	{
		std::string s;

		s += "Token { ";

		s += ANSI_CYAN ANSI_ITALIC "type";
		s += ANSI_RESET ANSI_CYAN " = ";
		s += ANSI_BOLD;
		s += token_type_to_str(type);
		s += ANSI_RESET ", ";

		s += ANSI_YELLOW ANSI_ITALIC "value";
		s += ANSI_RESET ANSI_YELLOW " = \"";
		s += ANSI_BOLD;
		s += value;
		s += ANSI_RESET ANSI_YELLOW "\"";
		s += ANSI_RESET ", ";

		s += ANSI_GREEN ANSI_ITALIC "line:col";
		s += ANSI_RESET ANSI_GREEN " = ";
		s += ANSI_BOLD;
		s += std::to_string(line);
		s += ":";
		s += std::to_string(col);
		s += ANSI_RESET ", ";

		s += ANSI_BRIGHT_RED ANSI_ITALIC "whitespace_before";
		s += ANSI_RESET ANSI_BRIGHT_RED " = ";
		s += ANSI_BOLD;
		s += bool_to_string(whitespace_before);

		s += ANSI_RESET;
		s += " } @ ";
		s += to_hex(this);

		return s;
	}
};

// A set of all whitespace characters.
std::unordered_set<char> whitespace_chars = {
	' ', '\t', '\n', '\r'
};

// A set of all special characters in the Tea language.
std::unordered_set<char> special_chars = {
	'(', ')', '{', '}', '[', ']', ',', ';', '"', '\'', '\\'
};

// A set of all operator characters in the Tea language.
// Operators can be compound, e.g. ++, --, +=, -=, etc.
std::unordered_set<char> operator_chars = {
	'+', '-', '*', '/', '%', '=', '<', '>', '&', '^', '|', '!', '~', '.', ':'
};

// A set of all basic types in the Tea language.
std::unordered_set<std::string> types = {
	"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64", "f32", "f64", "v0"
};

// A set of all keywords in the Tea language.
std::unordered_set<std::string> keywords = {
	"if", "else", "return", "while", "for", "break", "continue", "goto",
	"class", "syscall"
};

/**
 * @brief Enum containing the names of all valid tokens in the Tea language.
 */
enum Operator
{
	SCOPE_RESOLUTION,

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

	UNDEFINED_OPERATOR
};

/**
 * @brief Converts an `Operator` to a string.
 * @param op The operator to be converted.
 * @returns A string representation of the operator.
 */
const char *
op_to_str(Operator op)
{
	switch (op)
	{
	case SCOPE_RESOLUTION:
		return "SCOPE_RESOLUTION";
	case POSTFIX_INCREMENT:
		return "POSTFIX_INCREMENT";
	case POSTFIX_DECREMENT:
		return "POSTFIX_DECREMENT";
	case PREFIX_INCREMENT:
		return "PREFIX_INCREMENT";
	case PREFIX_DECREMENT:
		return "PREFIX_DECREMENT";
	case UNARY_PLUS:
		return "UNARY_PLUS";
	case UNARY_MINUS:
		return "UNARY_MINUS";
	case BITWISE_NOT:
		return "BITWISE_NOT";
	case LOGICAL_NOT:
		return "LOGICAL_NOT";
	case DEREFERENCE:
		return "DEREFERENCE";
	case ADDRESS_OF:
		return "ADDRESS_OF";
	case POINTER_TO_MEMBER:
		return "POINTER_TO_MEMBER";
	case DEREFERENCED_POINTER_TO_MEMBER:
		return "DEREFERENCED_POINTER_TO_MEMBER";
	case DIVISION:
		return "DIVISION";
	case REMAINDER:
		return "REMAINDER";
	case MULTIPLICATION:
		return "MULTIPLICATION";
	case ADDITION:
		return "ADDITION";
	case SUBTRACTION:
		return "SUBTRACTION";
	case LEFT_SHIFT:
		return "LEFT_SHIFT";
	case RIGHT_SHIFT:
		return "RIGHT_SHIFT";
	case LESS:
		return "LESS";
	case LESS_OR_EQUAL:
		return "LESS_OR_EQUAL";
	case GREATER:
		return "GREATER";
	case GREATER_OR_EQUAL:
		return "GREATER_OR_EQUAL";
	case EQUAL:
		return "EQUAL";
	case NOT_EQUAL:
		return "NOT_EQUAL";
	case BITWISE_AND:
		return "BITWISE_AND";
	case BITWISE_XOR:
		return "BITWISE_XOR";
	case BITWISE_OR:
		return "BITWISE_OR";
	case LOGICAL_AND:
		return "LOGICAL_AND";
	case LOGICAL_OR:
		return "LOGICAL_OR";
	case ASSIGNMENT:
		return "ASSIGNMENT";
	case SUM_ASSIGNMENT:
		return "SUM_ASSIGNMENT";
	case DIFFERENCE_ASSIGNMENT:
		return "DIFFERENCE_ASSIGNMENT";
	case QUOTIENT_ASSIGNMENT:
		return "QUOTIENT_ASSIGNMENT";
	case REMAINDER_ASSIGNMENT:
		return "REMAINDER_ASSIGNMENT";
	case PRODUCT_ASSIGNMENT:
		return "PRODUCT_ASSIGNMENT";
	case LEFT_SHIFT_ASSIGNMENT:
		return "LEFT_SHIFT_ASSIGNMENT";
	case RIGHT_SHIFT_ASSIGNMENT:
		return "RIGHT_SHIFT_ASSIGNMENT";
	case BITWISE_AND_ASSIGNMENT:
		return "BITWISE_AND_ASSIGNMENT";
	case BITWISE_XOR_ASSIGNMENT:
		return "BITWISE_XOR_ASSIGNMENT";
	case BITWISE_OR_ASSIGNMENT:
		return "BITWISE_OR_ASSIGNMENT";
	default:
		return "UNDEFINED";
	}
}

/**
 * Converts an `Operator` to an example string.
 * An example string consists of the operator used with dummy symbols as
 * the operand(s).
 *
 * Used in error messages.
 *
 * Examples:
 * * POSTFIX_INCREMENT: x++
 * * MULTIPLICATION: x * y
 *
 * @param op The operator to be converted.
 * @returns The example string.
 */
const char *
op_to_example_str(Operator op)
{
	switch (op)
	{
	case SCOPE_RESOLUTION:
		return "x::y";
	case POSTFIX_INCREMENT:
		return "x++";
	case POSTFIX_DECREMENT:
		return "x--";
	case PREFIX_INCREMENT:
		return "++x";
	case PREFIX_DECREMENT:
		return "--x";
	case UNARY_PLUS:
		return "+x";
	case UNARY_MINUS:
		return "-x";
	case BITWISE_NOT:
		return "~x";
	case LOGICAL_NOT:
		return "!x";
	case DEREFERENCE:
		return "*x";
	case ADDRESS_OF:
		return "&x";
	case POINTER_TO_MEMBER:
		return "x.y";
	case DEREFERENCED_POINTER_TO_MEMBER:
		return "x->y";
	case DIVISION:
		return "x / y";
	case REMAINDER:
		return "x % y";
	case MULTIPLICATION:
		return "x * y";
	case ADDITION:
		return "x + y";
	case SUBTRACTION:
		return "x - y";
	case LEFT_SHIFT:
		return "x << y";
	case RIGHT_SHIFT:
		return "x >> y";
	case LESS:
		return "x < y";
	case LESS_OR_EQUAL:
		return "x <= y";
	case GREATER:
		return "x > y";
	case GREATER_OR_EQUAL:
		return "x >= y";
	case EQUAL:
		return "x == y";
	case NOT_EQUAL:
		return "x != y";
	case BITWISE_AND:
		return "x & y";
	case BITWISE_XOR:
		return "x ^ y";
	case BITWISE_OR:
		return "x | y";
	case LOGICAL_AND:
		return "x && y";
	case LOGICAL_OR:
		return "x || y";
	case ASSIGNMENT:
		return "x = y";
	case SUM_ASSIGNMENT:
		return "x += y";
	case DIFFERENCE_ASSIGNMENT:
		return "x -= y";
	case QUOTIENT_ASSIGNMENT:
		return "x /= y";
	case REMAINDER_ASSIGNMENT:
		return "x %= y";
	case PRODUCT_ASSIGNMENT:
		return "x *= y";
	case LEFT_SHIFT_ASSIGNMENT:
		return "x <<= y";
	case RIGHT_SHIFT_ASSIGNMENT:
		return "x >>= y";
	case BITWISE_AND_ASSIGNMENT:
		return "x &= y";
	case BITWISE_XOR_ASSIGNMENT:
		return "x ^= y";
	case BITWISE_OR_ASSIGNMENT:
		return "x |= y";
	default:
		return "undefined";
	}
}

/**
 * @param op The operator to check.
 * @returns True if the operator is a prefix unary operator, false otherwise.
 * A prefix unary operator is one that takes a single operand on the left side.
 * Examples are ++x, &x, ~x, etc.
 * Used by the parser to determine the order to parse operators.
 */
bool
is_prefix_unary_operator(Operator op)
{
	switch (op)
	{
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

/**
 * @param op The operator to check.
 * @returns True if the operator is a postfix unary operator, false otherwise.
 * A postfix unary operator is one that takes a single operand on the right side.
 * Examples are x++ and x--.
 * Used by the parser to determine the order to parse operators.
 */
bool
is_postfix_unary_operator(Operator op)
{
	switch (op)
	{
	case POSTFIX_INCREMENT:
	case POSTFIX_DECREMENT:
		return true;

	default:
		return false;
	}
}

/**
 * @param op The operator to check.
 * @returns True if the operator is a binary operator, false otherwise.
 * A binary operator is one that takes two operands on both sides.
 * Examples are x * y, x->y, x % y, etc.
 * Used by the parser to determine the order to parse operators.
 */
bool
is_binary_operator(Operator op)
{
	switch (op)
	{
	case SCOPE_RESOLUTION:
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

enum Associativity
{
	LEFT_TO_RIGHT,
	RIGHT_TO_LEFT
};

// Used in the table below to make the code more readable.
typedef std::pair<std::vector<Operator>, Associativity> OperatorPrecedencePair;
#define mp std::make_pair<std::vector<Operator>, Associativity>

// The following table is used to determine the order
// in which operators are parsed.
// The first element of each pair is a vector of operators
// that are of the same precedence.
// The second element of each pair is the associativity
// of the operators in the vector.
std::vector<OperatorPrecedencePair> operator_precedence = {
	mp({ SCOPE_RESOLUTION }, LEFT_TO_RIGHT),
	mp({ POSTFIX_INCREMENT, POSTFIX_DECREMENT }, LEFT_TO_RIGHT),
	mp({ PREFIX_INCREMENT, PREFIX_DECREMENT, UNARY_PLUS, UNARY_MINUS,
		   BITWISE_NOT, LOGICAL_NOT, DEREFERENCE, ADDRESS_OF },
		RIGHT_TO_LEFT),
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
		   BITWISE_OR_ASSIGNMENT },
		RIGHT_TO_LEFT)
};

#undef mp

/**
 * @brief Converts a string to an operator type.
 * @param str The string to parse.
 * @param prefix A boolean indicating whether the operator is a prefix operator
 * or not. Ignored if the operator is a non-unary operator.
 * @returns The operator type.
 */
Operator
str_to_operator(const std::string &str, bool prefix = false)
{
	if (str.size() > 3)
		return UNDEFINED_OPERATOR;

	if (str == "::")
		return SCOPE_RESOLUTION;
	if (str == "++" && prefix)
		return PREFIX_INCREMENT;
	if (str == "--" && prefix)
		return PREFIX_DECREMENT;
	if (str == "++")
		return POSTFIX_INCREMENT;
	if (str == "--")
		return POSTFIX_DECREMENT;
	if (str == "+" && prefix)
		return UNARY_PLUS;
	if (str == "-" && prefix)
		return UNARY_MINUS;
	if (str == "~" && prefix)
		return BITWISE_NOT;
	if (str == "!" && prefix)
		return LOGICAL_NOT;
	if (str == "*" && prefix)
		return DEREFERENCE;
	if (str == "&" && prefix)
		return ADDRESS_OF;
	if (str == ".")
		return POINTER_TO_MEMBER;
	if (str == "->")
		return DEREFERENCED_POINTER_TO_MEMBER;
	if (str == "*")
		return MULTIPLICATION;
	if (str == "/")
		return DIVISION;
	if (str == "%")
		return REMAINDER;
	if (str == "+")
		return ADDITION;
	if (str == "-")
		return SUBTRACTION;
	if (str == "<<")
		return LEFT_SHIFT;
	if (str == ">>")
		return RIGHT_SHIFT;
	if (str == "<")
		return LESS;
	if (str == "<=")
		return LESS_OR_EQUAL;
	if (str == ">")
		return GREATER;
	if (str == ">=")
		return GREATER_OR_EQUAL;
	if (str == "==")
		return EQUAL;
	if (str == "!=")
		return NOT_EQUAL;
	if (str == "&")
		return BITWISE_AND;
	if (str == "^")
		return BITWISE_XOR;
	if (str == "|")
		return BITWISE_OR;
	if (str == "&&")
		return LOGICAL_AND;
	if (str == "||")
		return LOGICAL_OR;
	if (str == "=")
		return ASSIGNMENT;
	if (str == "+=")
		return SUM_ASSIGNMENT;
	if (str == "-=")
		return DIFFERENCE_ASSIGNMENT;
	if (str == "/=")
		return QUOTIENT_ASSIGNMENT;
	if (str == "%=")
		return REMAINDER_ASSIGNMENT;
	if (str == "*=")
		return PRODUCT_ASSIGNMENT;
	if (str == "<<=")
		return LEFT_SHIFT_ASSIGNMENT;
	if (str == ">>=")
		return RIGHT_SHIFT_ASSIGNMENT;
	if (str == "&=")
		return BITWISE_AND_ASSIGNMENT;
	if (str == "^=")
		return BITWISE_XOR_ASSIGNMENT;
	if (str == "|=")
		return BITWISE_OR_ASSIGNMENT;
	return UNDEFINED_OPERATOR;
}

/**
 * @brief Class that is responsible for tokenising a source file into a list of
 * tokens that can be parsed by the `Parser` class.
 */
struct Tokeniser
{
	// The file stream to tokenise.
	FileStreamReader reader;

	// The line number of the current token.
	size_t line;

	// The column number of the current token.
	size_t col;

	// Flag indicating whether the current token has a
	// whitespace before it or not.
	bool whitespace_before;

	/**
	 * @brief Adds a token to the list of tokens.
	 * Automatically sets the line and column number and
	 * the `whitespace_before` flag.
	 * @param type The type of the token.
	 * @param value The value of the token.
	 */
	void
	push_token(TokenType type, std::string value)
	{
		Token token = {
			.type              = type,
			.value             = value,
			.line              = line,
			.col               = col,
			.whitespace_before = whitespace_before
		};

		tokens.push_back(token);

		// Update the `whitespace_before` flag.

		whitespace_before = false;
	}

/**
 * @brief Macro that throws a tokenisation error.
 * @param message A format string for the message.
 * @param ... The arguments for the format string.
 */
#define throw_err(message, ...)                                                     \
	do                                                                          \
	{                                                                           \
		p_warn(stderr, "[ Tokenise Error ]: " message "\n", ##__VA_ARGS__); \
		p_warn(stderr, "At %ld:%ld\n", reader.pos.line, reader.pos.col);    \
		abort();                                                            \
	} while (0)

	// The list of tokens produced by the tokeniser.
	std::vector<Token> tokens;

	/**
	 * @brief Constructs a new Tokeniser object.
	 * @param input_file The file to tokenise.
	 */
	Tokeniser(FILE *input_file)
		: reader(input_file) {}

	/**
	 * @brief Prints the list of tokens in a human readable fashion.
	 */
	void
	print_tokens()
	{
		if (!_debug)
		{
			return;
		}

		p_trace(stdout, "\\\\\\ Tokens (%ld) \\\\\\\n\n", tokens.size());

		for (const Token &token : tokens)
		{
			p_trace(stdout, "%s\n", token.to_str().c_str());
		}

		p_trace(stdout, "\n/// Tokens ///\n");
	}

	/**
	 * @brief Tokenises the file.
	 * @returns The list of tokens.
	 */
	std::vector<Token>
	tokenise()
	{
		// The current character.
		int c;
		FilePos restore_pos;

		while (restore_pos = reader.read_char(c), c != EOF)
		{
			p_trace(stdout, "tokenise(): %c (%hhu)\n", c, (uint8_t) c);
			line = reader.pos.line;
			col  = reader.pos.col;

			if (c == EOF)
				break;

			if (whitespace_chars.count(c))
			{
				whitespace_before = true;
				continue;
			}

			// Handle comments.

			if (c == '/')
			{
				if (scan_comment())
				{
					continue;
				}
			}

			if (c == '#')
			{
				ignore_until_eol();
			}

			// Handle string literals.

			else if (c == '"')
			{
				push_token(LITERAL_STRING, scan_literal_string());
			}

			// Handle character literals.

			else if (c == '\'')
			{
				push_token(LITERAL_CHAR, std::string(1, scan_literal_char()));
			}

			// Handle special character.

			else if (special_chars.count(c))
			{
				push_token(SPECIAL_CHARACTER, std::string(1, c));
			}

			// Handle operators.

			else if (operator_chars.count(c))
			{
				push_token(OPERATOR, scan_operator(c));
			}

			// Handle number literals.

			else if ((c >= '0' && c <= '9') || c == '.')
			{
				push_token(LITERAL_NUMBER, scan_literal_number(c));
			}

			// Handle text (keywords and symbols).

			else if (is_alpha(c))
			{
				std::string text = scan_text(c);

				// If the token is a type.

				if (types.count(text))
					push_token(TYPE, text);

				// If the token is a keyword.

				else if (keywords.count(text))
					push_token(KEYWORD, text);

				// It's probably an identifier.

				else
					push_token(IDENTIFIER, text);
			}

			// Unknown character.

			else
				throw_err("Found unknown character '%c' (%hhu)", c, (uint8_t) c);
		}

		return tokens;
	}

	/**
	 * @brief Scans a comment.
	 * Currently only supports // comments.
	 * TODO: support / * * / comments. Typo is intentional because
	 * I cannot write such a comment in this comment because
	 * it would break out of this comment.
	 * @returns A boolean indicating if a comment was
	 * successfully scanned.
	 */
	bool
	scan_comment()
	{
		int next_char;
		FilePos restore_pos = reader.read_char(next_char);

		// If the next character is not a /,
		// it's not a // comment.
		// TODO: support /* comments */.

		if (next_char == '/')
		{
			ignore_until_eol();

			return true;
		}
		else
		{
			reader.restore(restore_pos);
			return false;
		}
	}

	/**
	 * @brief Ignores everything until the end of the line.
	 * Used for scanning comments.
	 */
	void
	ignore_until_eol()
	{
		int next_char;

		do
		{
			reader.read_char(next_char);
		} while (next_char != '\n' && next_char != EOF);
	}

	/**
	 * @brief Scans escape sequences.
	 * Used in string literal scanning.
	 * @returns The escape sequence as an ascii character.
	 * TODO: support unicode escape sequences.
	 */
	char
	scan_escape_sequence()
	{
		int c;
		reader.read_char(c);

		switch (c)
		{
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
			int xc1;
			reader.read_char(xc1);
			int xc2;
			reader.read_char(xc2);

			if (!is_hex(xc1) || !is_hex(xc2))
				throw_err("Invalid hexadecimal escape code");

			return hex_chars_to_byte(xc1, xc2);
		}

		default:
			throw_err("Invalid escape code");
		}
	}

	/**
	 * @brief Scans a string literal surrounded by double quotes.
	 * Converts escape sequences to their ascii characters.
	 * @returns The contents of the string literal.
	 */
	std::string
	scan_literal_string()
	{
		std::string s;
		int c;

		while (true)
		{
			reader.read_char(c);
			if (c == EOF)
				throw_err("Unexpected EOF");

			// Escape sequence.

			if (c == '\\')
			{
				s += scan_escape_sequence();
				continue;
			}

			// Stop on end of string.

			if (c == '"')
			{
				break;
			}

			// Add this character to the string.

			s += c;
		}

		return s;
	}

	/**
	 * @brief Scans a character literal surrounded by single quotes.
	 * Converts an escape sequence to its ascii character.
	 * @returns The contents of the character literal.
	 */
	char
	scan_literal_char()
	{
		int c;
		reader.read_char(c);
		if (c == EOF)
			throw_err("Unexpected EOF");

		// Escape sequence.

		if (c == '\\')
			c = scan_escape_sequence();

		int next;
		reader.read_char(next);

		// Expect a closing single quote.

		if (next == EOF)
			throw_err("Unexpected EOF");
		if (next != '\'')
			throw_err("Unexpected char '%c', expected ending quote", next);

		return c;
	}

	/**
	 * @brief Scans a number.
	 * Supports hexadecimal, octal, and decimal numbers.
	 * Supports floating point and integer numbers.
	 * @returns The number as a string.
	 */
	std::string
	scan_literal_number(char first_char)
	{
		std::string s;
		s += first_char;

		int c;
		FilePos restore_pos = reader.read_char(c);

		if (first_char == '0')
		{
			// If the first character is a '0',
			// we could have to deal with 0x or 0b prefix,
			// or a decimal seperator.

			// Decimal seperator for floating point number.

			if (c == '.')
			{
				do
				{
					s += '.';
					restore_pos = reader.read_char(c);
				} while (is_decimal(c));

				reader.restore(restore_pos);
				return s;
			}

			// Hexadecimal integer.

			if (c == 'x')
			{
				s.clear();

				while (restore_pos = reader.read_char(c), is_hex(c))
				{
					s += c;
				}

				reader.restore(restore_pos);

				return std::to_string(stoull(s, nullptr, 16));
			}

			// Binary integer.

			if (c == 'b')
			{
				s.clear();

				while (restore_pos = reader.read_char(c), is_binary(c))
				{
					s += c;
				}

				reader.restore(restore_pos);

				return std::to_string(stoull(s, nullptr, 2));
			}

			// Unrecognised prefix.

			if (c < '0' && c > '9' && c != '.')
				throw_err("Unexpected char '%c'", c);
		}

		// Decimal integer or floating point number.

		if ((c >= '0' && c <= '9') || c == '.')
		{
			do
			{
				s += c;
				restore_pos = reader.read_char(c);
			} while ((c >= '0' && c <= '9') || c == '.');
		}

		reader.restore(restore_pos);
		return s;
	}

	/**
	 * @brief Scans a symbol name, class name, function name,
	 * keyword name or any kind of text.
	 * @param first_char The first character of the text.
	 * Is there because this character is already consumed.
	 * @returns A string containing the text.
	 */
	std::string
	scan_text(char first_char)
	{
		std::string s;
		s += first_char;

		while (true)
		{
			int c;
			FilePos restore_pos = reader.read_char(c);

			if (!is_alphanumeric(c))
			{
				reader.restore(restore_pos);
				break;
			}

			s += c;
		}

		return s;
	}

	/**
	 * @brief Scans an operator.
	 * @param first_char The first character of the operator.
	 * Is there because this character is already consumed.
	 * @returns A string containing the operator.
	 */
	std::string
	scan_operator(char first_char)
	{
		// Check if the next two characters are
		// maybe also operators.

		int op_char_2;
		FilePos restore_pos_2 = reader.read_char(op_char_2);

		if (!operator_chars.count(op_char_2))
		{
			// Only the first character is an operator.

			reader.restore(restore_pos_2);
		}
		else
		{
			// The second character is an operator.
			// Check if the third is too.

			int op_char_3;
			FilePos restore_pos_3 = reader.read_char(op_char_3);

			if (!operator_chars.count(op_char_3))
			{
				// Only the first two characters
				// are operators.

				reader.restore(restore_pos_3);
			}
			else
			{
				// The third character is also an
				// operator. Check if we can form a
				// triple character operator.

				std::string op;
				op += first_char;
				op += op_char_2;
				op += op_char_3;

				if (str_to_operator(op) != UNDEFINED_OPERATOR)
					return op;
				else
					reader.restore(restore_pos_3);
			}

			// Check if we can form a double
			// character operator.

			std::string op;
			op += first_char;
			op += op_char_2;

			if (str_to_operator(op) != UNDEFINED_OPERATOR)
				return op;
			else
				reader.restore(restore_pos_2);
		}

		std::string op;
		op += first_char;
		return op;
	}
};

#endif