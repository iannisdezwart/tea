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

using namespace std;

template <typename intx_t>
string to_hex(intx_t num)
{
	stringstream stream;
	stream << "0x" << setfill('0') << setw(8) << hex << num;
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

#endif