#ifndef TEA_COMPILER_UTIL_HEADER
#define TEA_COMPILER_UTIL_HEADER

#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <iomanip>

static bool _debug;

#define p_trace(stream, ...)                          \
	do                                            \
	{                                             \
		if (_debug)                           \
			fprintf(stream, __VA_ARGS__); \
	} while (0)

#define p_warn(stream, ...)                   \
	do                                    \
	{                                     \
		fprintf(stream, __VA_ARGS__); \
	} while (0)

/**
 * @brief Macro that prints an error message and aborts the program.
 * Appends a newline to the error message.
 * @param message The message format string.
 * @param ... The arguments to the format string.
 */
#define err(message, ...)                                    \
	do                                                   \
	{                                                    \
		p_warn(stderr, message "\n", ##__VA_ARGS__); \
		abort();                                     \
	} while (0)

/**
 * @brief Macro that prints an error message at a line and column,
 * received by a token and aborts the program.
 * Appends a newline to the error message.
 * @param token The token to print the line and column of.
 * @param message The message format string.
 * @param ... The arguments to the format string.
 */
#define err_at_token(token, prefix, message, ...)                                                          \
	do                                                                                                 \
	{                                                                                                  \
		p_warn(stderr, "[ %s ]: " message "\n", prefix, ##__VA_ARGS__);                            \
		p_warn(stderr, "At %d:%d\n", static_cast<uint>(token.line), static_cast<uint>(token.col)); \
		abort();                                                                                   \
	} while (0)

/**
 * @brief Macro that prints a warning message.
 * Appends a newline to the warning message.
 * @param message The message format string.
 * @param ... The arguments to the format string.
 */
#define warn(message, ...)                                                   \
	do                                                                   \
	{                                                                    \
		p_warn(stderr, "[ warning ]: " message "\n", ##__VA_ARGS__); \
	} while (0)

/**
 * @brief Macro that evaluates to true if the character is an
 * alphabetical character (A-Z, a-z, $, _).
 * @param c The character to check.
 */
#define is_alpha(c) ((c) >= 'A' && (c) <= 'Z' || (c) >= 'a' && (c) <= 'z' \
	|| (c) == '$' || (c) == '_')

/**
 * @brief Macro that evaluates to true if the character is an
 * alphanumerical character (A-Z, a-z, 0-9, $, _).
 * @param c The character to check.
 */
#define is_alphanumeric(c) (is_alpha(c) || (c) >= '0' && (c) <= '9')

/**
 * @brief Macro that checks if a given character is an octal digit (0-7).
 * @param c The character to check.
 */
#define is_octal(c) (c >= '0' && c <= '7')

/**
 * @brief Macro that checks if a given character is a decimal digit (0-9).
 * @param c The character to check.
 */
#define is_decimal(c) (c >= '0' && c <= '9')

/**
 * @brief Macro that checks if a given character is a hexadecimal digit
 * (0-9, A-F, a-f).
 * @param c The character to check.
 */
#define is_hex(c) (c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f' \
	|| c >= '0' && c <= '9')

/**
 * @brief Macro that checks if a given character is a binary digit (0 or 1).
 * @param c The character to check.
 */
#define is_binary(c) (c == '0' || c == '1')

/**
 * @brief Converts a number to a hexadecimal string.
 * @tparam intx_t The number type.
 * @param num The number to convert.
 * @returns A string containing the hexadecimal representation of the number.
 */
template <typename intx_t>
std::string
to_hex(intx_t num)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(8) << std::hex << num;
	return stream.str();
}

/**
 * @brief Converts a single hex character to its corresponding number.
 * @param x The hex character to convert.
 * @returns An integer (0-15) corresponding to the hex character.
 */
uint8_t
hex_char_to_num(char x)
{
	if (x >= '0' && x <= '9')
		return x - '0';
	if (x >= 'a')
		return x - 'a' + 10;
	return x - 'A' + 10;
}

/**
 * @brief Converts two hex characters to a byte.
 * @param upper The upper hex character.
 * @param lower The lower hex character.
 * @returns The byte corresponding to the two hex characters.
 */
char
hex_chars_to_byte(char upper, char lower)
{
	return hex_char_to_num(upper) << 4 | hex_char_to_num(lower);
}

char
octal_chars_to_byte(char upper, char middle, char lower)
{
	return (upper - '0') << 6 | (middle - '0') << 3 | (lower - '0');
}

/**
 * Cast std::unique_ptr<T> to std::unique_ptr<U>.
 */
template <typename To, typename From>
std::unique_ptr<To>
static_unique_ptr_cast(std::unique_ptr<From> &&ptr)
{
	return std::unique_ptr<To> { static_cast<To *>(ptr.release()) };
}

#endif