#ifndef TEA_COMPILER_UTIL_HEADER
#define TEA_COMPILER_UTIL_HEADER

#include <bits/stdc++.h>

#define err(message, ...) do { \
	fprintf(stderr, message, ##__VA_ARGS__); \
	putc('\n', stderr); \
	abort(); \
} while (0)

#define err_at_token(token, prefix, message, ...) do { \
	fprintf(stderr, "[ %s ]: " message "\n", prefix, ##__VA_ARGS__); \
	fprintf(stderr, "At %ld:%ld\n", token.line, token.col); \
	abort(); \
} while (0)

#define warn(message, ...) do { \
	fprintf(stderr, "[ warning ]: " message "\n", ##__VA_ARGS__); \
} while (0)

#define is_alpha(c) (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' \
	|| c == '$' || c == '_')

#define is_alphanumeric(c) (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' \
	|| c == '$' || c == '_' || c >= '0' && c <= '9')

#define is_octal(c) (c >= '0' && c <= '7')

#define is_decimal(c) (c >= '0' && c <= '9')

#define is_hex(c) (c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f' \
	|| c >= '0' && c <= '9')

#define is_binary(c) (c == '0' || c == '1')

using namespace std;

template <typename intx_t>
string to_hex(intx_t num)
{
	stringstream stream;
	stream << setfill('0') << setw(8) << hex << num;
	return stream.str();
}

uint8_t hex_char_to_num(char x)
{
	if (x >= '0' && x <= '9') return x - '0';
	if (x >= 'a') return x - 'a' + 10;
	return x - 'A' + 10;
}

char hex_chars_to_byte(char upper, char lower)
{
	return hex_char_to_num(upper) << 4 | hex_char_to_num(lower);
}

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

string pad_start(const string& str, size_t len, char c)
{
	if (str.length() >= len) return str;
	return string(c, len - str.length()) + str;
}

bool compare_str(const string& str, const char *test)
{
	if (str.length() > strlen(test)) return true;
	return pad_start(str, strlen(test), '0') > test;
}

bool fits_uint8(const string& str)
{
	if (str[0] == '-') return false;
	if (compare_str(str, abs_max_uint8)) return false;
	return true;
}

bool fits_int8(const string& str)
{
	if (str[0] == '-') {
		if (compare_str(str.substr(1), abs_min_int8)) return false;
		return true;
	}

	if (compare_str(str, abs_max_int8)) return false;
	return true;
}

bool fits_uint16(const string& str)
{
	if (str[0] == '-') return false;
	if (compare_str(str, abs_max_uint16)) return false;
	return true;
}

bool fits_int16(const string& str)
{
	if (str[0] == '-') {
		if (compare_str(str.substr(1), abs_min_int16)) return false;
		return true;
	}

	if (compare_str(str, abs_max_int16)) return false;
	return true;
}

bool fits_uint32(const string& str)
{
	if (str[0] == '-') return false;
	if (compare_str(str, abs_max_uint32)) return false;
	return true;
}

bool fits_int32(const string& str)
{
	if (str[0] == '-') {
		if (compare_str(str.substr(1), abs_min_int32)) return false;
		return true;
	}

	if (compare_str(str, abs_max_int32)) return false;
	return true;
}

bool fits_uint64(const string& str)
{
	if (str[0] == '-') return false;
	if (compare_str(str, abs_max_uint64)) return false;
	return true;
}

bool fits_int64(const string& str)
{
	if (str[0] == '-') {
		if (compare_str(str.substr(1), abs_min_int64)) return false;
		return true;
	}

	if (compare_str(str, abs_max_int64)) return false;
	return true;
}

#endif