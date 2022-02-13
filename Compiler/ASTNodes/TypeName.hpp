#ifndef TEA_AST_NODE_TYPE_NAME_HEADER
#define TEA_AST_NODE_TYPE_NAME_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../compiler-state.hpp"
#include "../util.hpp"

struct TypeName : public ASTNode {
		Token type_token;
		std::vector<size_t> array_sizes;

		TypeName(const Token& type_token, std::vector<size_t>&& array_sizes)
			: type_token(type_token), array_sizes(std::move(array_sizes)),
				ASTNode(type_token, TYPE_NAME) {}

		void dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			callback(this, depth);
		}

		size_t pointer_depth()
		{
			return array_sizes.size();
		}

		std::string type_to_str()
		{
			std::string out;

			out += type_token.value;

			for (size_t i = 0; i < array_sizes.size(); i++) {
				if (array_sizes[i] == 0) {
					out += '*';
				} else {
					out += '[';
					out += to_string(array_sizes[i]);
					out += ']';
				}
			}

			return out;
		}

		std::string to_str()
		{
			std::string s = "TypeName { type = \"" + type_to_str() + "\" } @ "
				+ to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			if (compiler_state.classes.count(type_token.value)) {
				Class class_decl = compiler_state.classes[type_token.value];
				size_t byte_size = class_decl.byte_size;

				Type type(Type::USER_DEFINED_CLASS, byte_size, array_sizes);
				type.class_name = type_token.value;

				for (const Identifier& field : class_decl.fields) {
					type.fields.push_back(field.type);
				}

				return type;
			}

			return Type::from_string(type_token.value, array_sizes);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {}
};

#endif