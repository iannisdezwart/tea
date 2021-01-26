#ifndef TEA_PARSER_HEADER
#define TEA_PARSER_HEADER

#include <bits/stdc++.h>

using namespace std;

enum TokenType {
	DATATYPE
};

struct Token {
	TokenType type;
	string value;
};

#endif