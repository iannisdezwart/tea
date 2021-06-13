#ifndef TEA_AST_NODE_TYPE_IDENTIFIER_HEADER
#define TEA_AST_NODE_TYPE_IDENTIFIER_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"

using namespace std;

class TypeIdentifierPair : public ASTNode {
	public:
		Token type_token;
		uint8_t pointer_depth;
		Token identifier_token;

		TypeIdentifierPair(Token type_token, uint8_t pointer_depth,
			Token identifier_token)
				: type_token(type_token), pointer_depth(pointer_depth),
					identifier_token(identifier_token), ASTNode(identifier_token)
		{
			type = TYPE_IDENTIFIER_PAIR;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth) {
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			callback(this, depth);
		}

		string get_identifier_name()
		{
			return identifier_token.value;
		}

		string to_str()
		{
			string s = "TypeIdentifierPair { type = \"" + type_token.value + "\", "
				"identifier = \"" + identifier_token.value +
				"\", pointer_depth = \"" + to_string(pointer_depth) + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			if (compiler_state.classes.count(type_token.value)) {
				Class class_decl = compiler_state.classes[type_token.value];
				size_t byte_size = class_decl.byte_size;
				return Type(Type::USER_DEFINED_CLASS, byte_size, pointer_depth);
			}

			return Type::from_string(type_token.value, pointer_depth);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {}
};

#endif