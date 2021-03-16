#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include <bits/stdc++.h>

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../util.hpp"
#include "TypeIdentifierPair.hpp"

using namespace std;

class VariableDeclaration : public ASTNode {
	public:
		TypeIdentifierPair *type_and_id_pair;
		ASTNode *expression;

		VariableDeclaration(
			TypeIdentifierPair *type_and_id_pair,
			ASTNode *expression
		) {
			this->type_and_id_pair = type_and_id_pair;
			this->expression = expression;
			type = VARIABLE_DECLARATION;

			#ifdef PARSER_VERBOSE
			print("Created");
			#endif
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			#ifdef PARSER_VERBOSE
			print("dfs");
			#endif

			if (type_and_id_pair != NULL)
				type_and_id_pair->dfs(callback, depth + 1);

			if (expression != NULL)
				expression->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s = "VariableDeclaration {} @ " + to_hex((size_t) this);
			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			return type_and_id_pair->get_type(compiler_state);
		}

		void compile(Assembler& assembler, CompilerState& compiler_state) {
			if (expression != NULL) {
				// Moves result into R_ACCUMULATOR

				expression->compile(assembler, compiler_state);

				// Move R_ACCUMULATOR into the offset for the variable

				string id_name = type_and_id_pair->get_identifier_name();
				Variable& var = compiler_state.locals[id_name];
				Type& type = var.type;
				uint64_t offset = var.offset;
				uint64_t var_size = type.byte_size();

				switch (var_size) {
					case 1:
						assembler.move_reg_into_frame_offset_8(R_ACCUMULATOR_0_ID, offset);
						break;

					case 2:
						assembler.move_reg_into_frame_offset_16(R_ACCUMULATOR_0_ID, offset);
						break;

					case 4:
						assembler.move_reg_into_frame_offset_32(R_ACCUMULATOR_0_ID, offset);
						break;

					case 8:
						assembler.move_reg_into_frame_offset_64(R_ACCUMULATOR_0_ID, offset);
						break;
				}
			}
		}
};

#endif