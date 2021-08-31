#ifndef TEA_DEBUGGER_UTIL_HEADER
#define TEA_DEBUGGER_UTIL_HEADER

#include <bits/stdc++.h>

using namespace std;

template <typename Type>
struct PtrCmp {
	bool operator()(const Type *lhs, const Type *rhs) const
	{
		return lhs < rhs;
	}
};

using PtrSet = set<uint8_t *, PtrCmp<uint8_t>>;

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

uint8_t *read_ptr(string&& addr_str)
{
	bool hexadecimal = false;
	uint64_t ptr;

	// If the address starts with "0x" or "x", use hex format

	if (addr_str.size() >= 1 && addr_str[0] == 'x') {
		addr_str = addr_str.substr(1);
		hexadecimal = true;
	} else if (addr_str.size() >= 2 && addr_str[0] == '0' && addr_str[1] == 'x') {
		addr_str = addr_str.substr(2);
		hexadecimal = true;
	}

	// Read the address

	stringstream ss;

	if (hexadecimal) {
		ss << hex;
	}

	ss << addr_str;
	ss >> ptr;

	return (uint8_t *) ptr;
}

#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n')

#endif