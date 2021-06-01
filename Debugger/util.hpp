#ifndef TEA_DEBUGGER_UTIL_HEADER
#define TEA_DEBUGGER_UTIL_HEADER

#include <bits/stdc++.h>

using namespace std;

bool starts_with(const string& str, const string& search)
{
	if (search.size() > str.size()) return false;

	for (size_t i = 0; i < search.size(); i++) {
		if (str[i] != search[i]) return false;
	}

	return true;
}

template <typename intx_t>
string to_hex_str(intx_t num)
{
	stringstream ss;
	ss << hex << num;
	return ss.str();
}

#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n')

#endif