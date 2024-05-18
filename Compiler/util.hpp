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
#define err_at_token(token, prefix, message, ...)                               \
	do                                                                      \
	{                                                                       \
		p_warn(stderr, "[ %s ]: " message "\n", prefix, ##__VA_ARGS__); \
		p_warn(stderr, "At %ld:%ld\n", token.line, token.col);          \
		abort();                                                        \
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

// Below are the minimum and maximum values that fit in a given
// integer type. These are used to check if a literal number fits
// in the range of the integer type.
// They are stored as char arrays, which we will be able to compare
// with the literal number using alphabetical ordering.
// ASCII is great <3

const char *abs_max_uint8 = "255";

const char *abs_max_int8 = "127";
const char *abs_min_int8 = "128";

const char *abs_max_uint16 = "65535";

const char *abs_max_int16 = "32767";
const char *abs_min_int16 = "32768";

const char *abs_max_uint32 = "4294967295";

const char *abs_max_int32 = "2147483647";
const char *abs_min_int32 = "2147483648";

const char *abs_max_uint64 = "18446744073709551615";

const char *abs_max_int64 = "9223372036854775807";
const char *abs_min_int64 = "9223372036854775808";

/**
 * @brief Pads the start of a string with a given character to a given length.
 * @param str The string to pad.
 * @param len The maximum length to pad the string to.
 * @param c The character to pad with.
 * @returns A new padded string.
 */
std::string
pad_start(const std::string &str, size_t len, char c)
{
	if (str.length() >= len)
		return str;
	return std::string(c, len - str.length()) + str;
}

/**
 * @brief Compares a string to a given character array using alphabetical
 * ordering. The string is first padded with '0's to the length of the
 * character array.
 * @param str The string.
 * @param test The character array to compare to.
 * @returns A boolean if the padded string is greater than the character array.
 */
bool
compare_str(const std::string &str, const char *test)
{
	if (str.length() > std::strlen(test))
		return true;
	return pad_start(str, std::strlen(test), '0') > test;
}

/**
 * @brief Checks if a given literal fits in an unsigned 8-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in a u8.
 */
bool
fits_uint8(const std::string &str)
{
	if (str[0] == '-')
		return false;
	if (compare_str(str, abs_max_uint8))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in a signed 8-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an i8.
 */
bool
fits_int8(const std::string &str)
{
	if (str[0] == '-')
	{
		if (compare_str(str.substr(1), abs_min_int8))
			return false;
		return true;
	}

	if (compare_str(str, abs_max_int8))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in an unsigned 16-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in a u16.
 */
bool
fits_uint16(const std::string &str)
{
	if (str[0] == '-')
		return false;
	if (compare_str(str, abs_max_uint16))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in a signed 16-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an i16.
 */
bool
fits_int16(const std::string &str)
{
	if (str[0] == '-')
	{
		if (compare_str(str.substr(1), abs_min_int16))
			return false;
		return true;
	}

	if (compare_str(str, abs_max_int16))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in an unsigned 32-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in a u32.
 */
bool
fits_uint32(const std::string &str)
{
	if (str[0] == '-')
		return false;
	if (compare_str(str, abs_max_uint32))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in a signed 32-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an i32.
 */
bool
fits_int32(const std::string &str)
{
	if (str[0] == '-')
	{
		if (compare_str(str.substr(1), abs_min_int32))
			return false;
		return true;
	}

	if (compare_str(str, abs_max_int32))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in an unsigned 64-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in a u64.
 */
bool
fits_uint64(const std::string &str)
{
	if (str[0] == '-')
		return false;
	if (compare_str(str, abs_max_uint64))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in a signed 64-bit integer.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an i64.
 */
bool
fits_int64(const std::string &str)
{
	if (str[0] == '-')
	{
		if (compare_str(str.substr(1), abs_min_int64))
			return false;
		return true;
	}

	if (compare_str(str, abs_max_int64))
		return false;
	return true;
}

/**
 * @brief Checks if a given literal fits in a 32-bit floating point number.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an f32.
 */
bool
fits_float32(const std::string &str)
{
	try
	{
		std::stof(str);
		return true;
	}
	catch (std::invalid_argument &e)
	{
		return false;
	}
}

/**
 * @brief Checks if a given literal fits in a 64-bit floating point number.
 * @param str The literal string.
 * @returns A boolean indicating whether the literal fits in an f64.
 */
bool
fits_float64(const std::string &str)
{
	try
	{
		std::stod(str);
		return true;
	}
	catch (std::invalid_argument &e)
	{
		return false;
	}
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