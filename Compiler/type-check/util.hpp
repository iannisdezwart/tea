#ifndef TEA_TYPE_CHECK_UTIL_HEADER
#define TEA_TYPE_CHECK_UTIL_HEADER

#include "Compiler/type.hpp"

/**
 * @brief Enum for the different types of identifiers:
 * globals, functions, parameters and locals.
 */
enum struct IdentifierKind
{
	UNDEFINED,
	GLOBAL,
	FUNCTION,
	PARAMETER,
	LOCAL,
};

/**
 * @brief Structure for an identifier definition.
 * Holds the name and type.
 */
struct IdentifierDefinition
{
	// The id of this identifier.
	uint id;

	// The data type of this identifier.
	Type type;

	IdentifierDefinition() {}
	IdentifierDefinition(uint id, Type type)
		: id(id), type(std::move(type)) {}
};

/**
 * @brief Structure for a variable definition.
 * Holds the name, type and offset.
 * Depending on whether the variable is a global, local or parameter,
 * the offset is either relative to the top of the stack,
 * or the start of the current stack frame.
 */
struct VariableDefinition
{
	// The offset of this variable.
	// Depending on whether the variable is a global, local or parameter,
	// the offset is either relative to the top of the stack,
	// or the start of the current stack frame.
	uint offset;

	// Holds the name and type of this variable.
	IdentifierDefinition id;

	VariableDefinition() {}
	VariableDefinition(uint id, Type &type, uint offset)
		: offset(offset),
		  id(id, type) {}
};

/**
 * @brief Structure for a function signature.
 * Contains the name and return type, as well as the parameter identifiers.
 */
struct FunctionSignature
{
	// Holds the name and return type of this function.
	IdentifierDefinition id;

	// Holds the identifiers of the parameters of this function.
	std::vector<IdentifierDefinition> parameters;

	FunctionSignature() {}
	FunctionSignature(uint fn_id, Type &return_type)
		: id(fn_id, return_type) {}

	void
	add_parameter(uint param_id, const Type &param_type)
	{
		parameters.push_back(IdentifierDefinition(param_id, param_type));
	}
};

/**
 * @brief Structure for a class definition.
 * Holds the byte size of the class,
 * as well as the identifiers and methods of the class.
 * Does not contain the name of the class,
 * since that will be the key to the class in the `type_check_state.classes` map.
 */
struct ClassDefinition
{
	// The byte size of the class.
	uint byte_size;

	// A list of all fields in the class.
	std::vector<IdentifierDefinition> fields;

	ClassDefinition() {}
	ClassDefinition(uint byte_size)
		: byte_size(byte_size) {}

	void
	add_field(uint field_id, const Type &field_type)
	{
		fields.push_back(IdentifierDefinition(field_id, field_type));
	}

	const Type &
	get_field_type(uint field_id) const
	{
		for (const IdentifierDefinition &field : fields)
		{
			if (field.id == field_id)
				return field.type;
		}

		err("Class field \"%u\" not found", field_id);
	}

	uint
	get_field_offset(uint field_id, const std::vector<uint> &extra_data) const
	{
		uint offset = 0;

		for (const IdentifierDefinition &field : fields)
		{
			if (field.id == field_id)
				return offset;

			offset += field.type.storage_size(extra_data);
		}

		err("Class field \"%u\" not found", field_id);
	}

	bool
	has_field(uint field_id) const
	{
		for (const IdentifierDefinition &field : fields)
		{
			if (field.id == field_id)
				return true;
		}

		return false;
	}
};

#endif