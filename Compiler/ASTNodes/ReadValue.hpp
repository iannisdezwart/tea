#ifndef TEA_READ_VALUE_HEADER
#define TEA_READ_VALUE_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/tokeniser.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"

bool
is_read_value(ASTNode *node)
{
	switch (node->node_type)
	{
	case BINARY_OPERATION:
	case UNARY_OPERATION:
	case LITERAL_CHAR_EXPRESSION:
	case LITERAL_NUMBER_EXPRESSION:
	case LITERAL_STRING_EXPRESSION:
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

struct ReadValue : public ASTNode
{
	ReadValue(Token accountable_token, ASTNodeType node_type)
		: ASTNode(std::move(accountable_token), node_type) {}

	virtual ~ReadValue() {}

	/**
	 *  Gets the value of this expression and ignores it.
	 */
	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t result_reg = assembler.get_register();
		get_value(assembler, result_reg);
		assembler.free_register(result_reg);
	}

	/**
	 *  Gets the value of this expression and puts it into result_reg.
	 */
	virtual void
	get_value(Assembler &assembler, uint8_t result_reg)
		const = 0;

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

constexpr int READ_VALUE_SIZE = sizeof(ReadValue);

#endif