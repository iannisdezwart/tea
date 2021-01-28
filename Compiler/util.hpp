#ifndef TEA_UTIL_HEADER
#define TEA_UTIL_HEADER

#include <bits/stdc++.h>

#define err(message, ...) do { \
	fprintf(stderr, message, ##__VA_ARGS__); \
	putc('\n', stderr); \
	exit(0); \
} while (0)

#define is_alpha(c) (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' \
	|| c == '$' || c == '_')

#define is_alphanumeric(c) (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' \
	|| c == '$' || c == '_' || c >= '0' && c <= '9')

using namespace std;

template <typename intx_t>
string to_hex(intx_t num)
{
	stringstream stream;
	stream << "0x" << setfill('0') << setw(8) << hex << num;
	return stream.str();
}

#endif