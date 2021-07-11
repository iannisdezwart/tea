#ifndef TEA_WRITE_VALUE_HEADER
#define TEA_WRITE_VALUE_HEADER

#include <bits/stdc++.h>

#include "ReadValue.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"

bool is_write_value(ASTNode *node)
{
	switch (node->type) {
		case IDENTIFIER_EXPRESSION:
		case MEMBER_EXPRESSION:
			return true;

		default:
			return false;
	}
}

class WriteValue : public ReadValue
{
	public:
		WriteValue(const Token& accountable_token, ASTNodeType type)
			: ReadValue(accountable_token, type) {}

		/**
		 *  Stores the value in R_ACCUMULATOR_0 into this value.
		 */
		virtual void store(Assembler& assembler, CompilerState& compiler_state) = 0;

		/**
		 *  Gets data about the location of the value.
		 */
		virtual LocationData get_location_data(CompilerState& compiler_state) = 0;

		/**
		 *  Casts an ASTNode into a WriteValue.
		 */
		static WriteValue *cast(ASTNode *node)
		{
			if (!is_write_value(node)) {
				err_at_token(node->accountable_token,
					"Value Type Error",
					"Expected a WriteValue\n"
					"This value is not writable");
			}

			return (WriteValue *) node;
		}
};

#endif