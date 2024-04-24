#ifndef TEA_READ_VALUE_HEADER
#define TEA_READ_VALUE_HEADER

#include "ASTNode.hpp"
#include "../tokeniser.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"

bool
is_read_value(ASTNode *node)
{
	switch (node->type)
	{
	case BINARY_OPERATION:
	case UNARY_OPERATION:
	case LITERAL_CHAR_EXPRESSION:
	case LITERAL_NUMBER_EXPRESSION:
	case LITERAL_STRING_EXPRESSION:
	case INIT_LIST:
	case ASSIGNMENT_EXPRESSION:
	case FUNCTION_CALL:
	case IDENTIFIER_EXPRESSION:
	case MEMBER_EXPRESSION:
	case CAST_EXPRESSION:
	case OFFSET_EXPRESSION:
		return true;

	default:
		return false;
	}
}

class ReadValue : public ASTNode
{
	public:

	ReadValue(const Token &accountable_token, ASTNodeType type)
		: ASTNode(accountable_token, type) {}

	/**
	 *  Gets the value of this expression and ignores it.
	 */
	void
	compile(Assembler &assembler, CompilerState &compiler_state)
	{
		uint8_t result_reg = assembler.get_register();
		get_value(assembler, compiler_state, result_reg);
		assembler.free_register(result_reg);
	}

	/**
	 *  Gets the value of this expression and puts it into result_reg.
	 */
	virtual void
	get_value(Assembler &assembler, CompilerState &compiler_state, uint8_t result_reg) = 0;

	/**
	 *  Casts an ASTNode into a ReadValue
	 */
	static ReadValue *
	cast(ASTNode *node)
	{
		if (!is_read_value(node))
		{
			err_at_token(node->accountable_token,
				"Value Type Error",
				"Expected a ReadValue\n"
				"This value is not readable");
		}

		return (ReadValue *) node;
	}
};

#endif