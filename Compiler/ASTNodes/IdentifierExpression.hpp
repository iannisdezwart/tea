#ifndef TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER
#define TEA_AST_NODE_IDENTIFIER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"

using namespace std;

class IdentifierExpression : public ASTNode {
	public:
		Token identifier_token;

		IdentifierExpression(Token identifier_token)
			: identifier_token(identifier_token), ASTNode(identifier_token)
		{
			type = IDENTIFIER_EXPRESSION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			callback(this, depth);
		}

		string to_str()
		{
			string s = "IdentifierExpression { identifier = \""
				+ identifier_token.value + "\" } @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			string id_name = identifier_token.value;
			Type type = compiler_state.get_type_of_identifier(id_name);

			if (type == Type::UNDEFINED)
				err_at_token(identifier_token,
					"Identifier has unknown kind",
					"Identifier: %s. this might be a bug in the compiler",
					id_name.c_str());

			return type;
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			string id_name = identifier_token.value;
			IdentifierKind id_kind = compiler_state.get_identifier_kind(id_name);

			if (id_kind == IdentifierKind::UNDEFINED)
				err_at_token(identifier_token,
					"Reference to undeclared variable",
					"Identifier %s was referenced, but not declared",
					id_name.c_str());

			int64_t offset;
			uint64_t var_size;

			switch (id_kind) {
				case IdentifierKind::LOCAL:
				{
					Variable& var = compiler_state.locals[id_name];
					Type& type = var.type;
					offset = var.offset;
					var_size = type.byte_size();
					break;
				}

				case IdentifierKind::PARAMETER:
				{
					Variable& var = compiler_state.parameters[id_name];
					Type& type = var.type;
					offset = -compiler_state.parameters_size + var.offset
						- 16 - CPU::stack_frame_size;
					var_size = type.byte_size();
					break;
				}

				case IdentifierKind::GLOBAL:
				{
					Variable& var = compiler_state.globals[id_name];
					Type& type = var.type;
					offset = var.offset;
					var_size = type.byte_size();
					break;
				}

				default:
					err_at_token(identifier_token,
						"Identifier has unknown kind",
						"Identifier: %s. this might be a bug in the compiler",
						id_name.c_str());
			}

			switch (var_size) {
				case 1:
					assembler.move_frame_offset_8_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 2:
					assembler.move_frame_offset_16_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 4:
					assembler.move_frame_offset_32_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				case 8:
					assembler.move_frame_offset_64_into_reg(offset, R_ACCUMULATOR_0_ID);
					break;

				default:
					err_at_token(identifier_token,
					"Variable doesn't fit in register",
					"Support for this is not implemented yet");
			}
		}
};

#endif