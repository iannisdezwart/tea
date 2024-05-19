#ifndef TEA_WRITE_VALUE_HEADER
#define TEA_WRITE_VALUE_HEADER

#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"

bool
is_write_value(ASTNode *node)
{
	switch (node->node_type)
	{
	case IDENTIFIER_EXPRESSION:
	case MEMBER_EXPRESSION:
	case UNARY_OPERATION:
	case OFFSET_EXPRESSION:
		return true;

	default:
		return false;
	}
}

struct WriteValue : public ReadValue
{
	LocationData location_data;

	WriteValue(CompactToken accountable_token, ASTNodeType type)
		: ReadValue(std::move(accountable_token), type) {}

	virtual ~WriteValue() {}

	/**
	 *  Stores the value in value_reg into this value.
	 */
	virtual void
	store(Assembler &assembler, uint8_t value_reg)
		const = 0;

	/**
	 *  Casts an ASTNode into a WriteValue.
	 */
	static WriteValue *
	cast(ASTNode *node)
	{
		if (!is_write_value(node))
		{
			err_at_token(node->accountable_token,
				"Value Type Error",
				"Expected a WriteValue\n"
				"This value is not writable");
		}

		return (WriteValue *) node;
	}
};

#endif