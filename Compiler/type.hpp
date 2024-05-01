#ifndef TEA_TYPE_HEADER
#define TEA_TYPE_HEADER

#include "Compiler/util.hpp"
#include "Compiler/debugger-symbols.hpp"

/**
 * Class that represents a data type of a variable.
 *
 * A type is specified by:
 * * its subtype (unsigned int, signed int, user defined class, or init list)
 * * the size of the type in bytes
 * * the "pointeryness" of the type, specified by the `array_sizes` field
 *
 * This class has lot's of handy methods to get additional information about
 * the data type it corresponds.
 */
struct Type
{
	/**
	 * @brief The different subtypes of a type.
	 */
	enum Value : uint8_t
	{
		UNDEFINED,
		UNSIGNED_INTEGER,
		SIGNED_INTEGER,
		USER_DEFINED_CLASS,
		INIT_LIST
	};

	// The subtype of the type.
	Value value;

	// The size of the type in bytes.
	size_t size;

	// The array dimensions of this type.
	// The size of this vector corresponds with
	// the "depth" of the type.
	// An array dimension of 0 indicates it is a pointer.
	// An array dimension of n indicates it is an array of
	// n elements.
	// This vector will be empty in case of a non-array,
	// non-pointer type.
	//
	// Examples:
	//   * u64* -> { 0 }
	//   * u64[3] -> { 3 }
	//   * u64[3][4] -> { 3, 4 }
	//   * u64[3]*[5] -> { 3, 0, 5 }
	std::vector<size_t> array_sizes;

	// If the subtype is a user-defined class,
	// this field will hold the name of the class.
	std::string class_name;

	// If the subtype is a user-defined class or an init list,
	// this field will hold the children types.
	std::vector<Type> fields;

	// This boolean indicates if the type is a literal.
	// In other words, if it had a specified value at compile time.
	bool is_literal = false;

	// If the type is a literal, this field will hold the value.
	std::string *literal_value;

	/**
	 * @brief Default constructor.
	 * Sets the type to undefined, so this has to be updated later.
	 * It's here to support putting Type objects in a vector etc.
	 */
	Type()
		: value(UNDEFINED) {}

	/**
	 * @brief Constructs a new Type object
	 * @param value The subtype of the type.
	 * @param size The size of the type in bytes.
	 */
	Type(Value value, size_t size)
		: value(value), size(size) {}

	/**
	 * @brief Constructs a new Type object.
	 * @param value The subtype of the type.
	 * @param size The size of the type in bytes.
	 * @param array_sizes Initialises the `array_sizes` field.
	 * See this field for reference.
	 */
	Type(Value value, size_t size, const std::vector<size_t> &array_sizes)
		: value(value), size(size), array_sizes(array_sizes) {}

	/**
	 * Returns the pointer depth of this type.
	 *
	 * Examples:
	 *   * u64* -> 1
	 *   * u64[3][4][5] -> 3
	 *   * u64[3]*[5] -> 3
	 */
	size_t
	pointer_depth() const
	{
		return array_sizes.size();
	}

	// Allow switch comparisons

	operator Value() const
	{
		return value;
	}

	// Don't allow conversion to boolean

	explicit
	operator bool() = delete;

	/**
	 * @brief Compares two types with each other for equality.
	 * @param other The other type.
	 * @returns True if the types have the same subtype
	 * and array dimensions, false otherwise.
	 */
	constexpr bool
	operator==(const Type &other) const
	{
		return value == other.value && size == other.size
			&& pointer_depth() == other.pointer_depth();
	}

	/**
	 * @brief Compares two types with each other for unequality.
	 * @param other The other type.
	 * @returns True if the types don't have the same subtype
	 * and array dimensions, false otherwise.
	 */
	constexpr bool
	operator!=(const Type &other) const
	{
		return value != other.value || size != other.size
			|| pointer_depth() != other.pointer_depth();
	}

	/**
	 * @brief Compares the subtype of two types for equality.
	 * @param other_value The other subtype.
	 * @returns True if the subtypes are equal, false otherwise.
	 */
	constexpr bool
	operator==(Type::Value other_value) const
	{
		return value == other_value;
	}

	/**
	 * @brief Compares the subtype of two types for unequality.
	 * @param other_value The other subtype.
	 * @returns True if the subtypes are not equal, false otherwise.
	 */
	constexpr bool
	operator!=(Type::Value other_value) const
	{
		return value != other_value;
	}

	/**
	 * @param deref_dep The dereference depth.
	 * This will be decremented from the pointer depth. Optional.
	 * @returns The byte size of the type.
	 * If the type is a pointer, this will be 8 bytes,
	 * the size of a pointer.
	 * If the type is not a pointer, this will be the byte size
	 * of the type.
	 */
	size_t
	byte_size(size_t deref_dep = 0) const
	{
		return (pointer_depth() - deref_dep > 0) ? 8 : size;
	}

	/**
	 * Returns the pointed byte size of the data type.
	 *
	 * Examples:
	 *   * u64 -> 8
	 *   * u32 -> 4
	 *   * u32[4][4][4] -> 4
	 *   * u32** -> 4
	 */
	size_t
	pointed_byte_size() const
	{
		return size;
	}

	/**
	 * @return Returns the type this data type is pointing to.
	 * This type is computed by discarding the `array_sizes`.
	 */
	Type
	pointed_type() const
	{
		if (array_sizes.size() == 0)
		{
			err("Compiler error: tried dereferencing non-pointer Type %s",
				to_str().c_str());
		}

		Type type = *this;
		type.array_sizes.erase(type.array_sizes.begin());
		return type;
	}

	/**
	 * @brief Computes the storage size of a type.
	 * Used to calculate the size of a (multidimensional) array.
	 * @returns The storage size of the type.
	 */
	size_t
	storage_size() const
	{
		// If the type is not a pointer, simply return the size.

		if (pointer_depth() == 0)
			return size;

		size_t n_members = 1;
		size_t dim       = 0;

		// Compute the dimension of the array and the
		// number of members it has.
		// To compute these, we loop over the `array_sizes`
		// until we find a size that is zero, indicating
		// a pointer.

		for (size_t i = pointer_depth(); i != 0; i--)
		{
			if (array_sizes[i - 1] == 0)
				break;
			n_members *= array_sizes[i - 1];
			dim++;
		}

		// The storage size can be calculated by multiplying
		// the number of members by the byte size of the
		// remainder of the type.
		// If the remainder of the type is empty
		// (for example, if the type is u64[3][4]),
		// the byte size is simply the size of the type (8).

		return n_members * byte_size(dim);
	}

	/**
	 * @returns A boolean indicating whether this type is an array.
	 * This is done by looking at the last element of the
	 * `array_sizes` vector. If it is non-zero,
	 * the type is an array. If it is zero or doesn't exist,
	 * the type is a pointer or a non-pointer.
	 */
	bool
	is_array() const
	{
		if (array_sizes.size() == 0)
			return false;
		return array_sizes.back() != 0;
	}

	/**
	 * @returns A boolean indicating whether this type is a class.
	 * A pointer to a class is not a class.
	 */
	bool
	is_class() const
	{
		if (value != Type::USER_DEFINED_CLASS)
			return false;
		return pointer_depth() == 0;
	}

	/**
	 * @returns A boolean indicating whether this type is an integer.
	 * This is done by checking if the type is a signed or unsigned integer.
	 */
	bool
	is_integer() const
	{
		return value == Type::SIGNED_INTEGER || value == Type::UNSIGNED_INTEGER;
	}

	/**
	 * @brief Converts a string to a type.
	 * Only the standard primitive Tea types are supported:
	 * u8, i8, u16, u32, i32, u64, i64, f32, f64, void.
	 * @param str The string to convert.
	 * @param array_sizes The `array_sizes` of the type.
	 * @returns The type parsed from the string.
	 */
	static Type
	from_string(std::string str, const std::vector<size_t> &array_sizes)
	{
		if (str == "u8")
			return Type(Type::UNSIGNED_INTEGER, 1, array_sizes);

		if (str == "i8")
			return Type(Type::SIGNED_INTEGER, 1, array_sizes);

		if (str == "u16")
			return Type(Type::UNSIGNED_INTEGER, 2, array_sizes);

		if (str == "i16")
			return Type(Type::SIGNED_INTEGER, 2, array_sizes);

		if (str == "u32")
			return Type(Type::UNSIGNED_INTEGER, 4, array_sizes);

		if (str == "i32")
			return Type(Type::SIGNED_INTEGER, 4, array_sizes);

		if (str == "u64")
			return Type(Type::UNSIGNED_INTEGER, 8, array_sizes);

		if (str == "i64")
			return Type(Type::SIGNED_INTEGER, 8, array_sizes);

		if (str == "void")
			return Type(Type::UNSIGNED_INTEGER, 0, array_sizes);

		err("Wasn't able to convert \"%s\" to a Type", str.c_str());
	}

	/**
	 * @returns A boolean indicating whether the type
	 * fits in a register. A type only fits in a register
	 * if it is a primitive type.
	 */
	bool
	fits_in_register()
	{
		return (value == Type::SIGNED_INTEGER || value == Type::UNSIGNED_INTEGER)
			&& size <= 8;
	}

	/**
	 * @returns A boolean indicating whether the type fits within
	 * another type. This means that this type can be promoted
	 * to the other type.
	 * @param type The other type.
	 */
	bool
	fits(const Type &type) const
	{
		// Primitives

		if (value == Type::UNSIGNED_INTEGER || value == Type::SIGNED_INTEGER)
		{
			// If the other type is not a primitive,
			// it will definitely not fit.

			if (type.value != Type::UNSIGNED_INTEGER && type.value != Type::SIGNED_INTEGER)
			{
				return false;
			}

			// If this type is a literal, check if we can
			// fit it without overflowing the type.

			if (is_literal)
			{
				if (type.value == Type::UNSIGNED_INTEGER && type.size == 1)
				{
					return fits_uint8(*literal_value);
				}

				if (type.value == Type::SIGNED_INTEGER && type.size == 1)
				{
					return fits_int8(*literal_value);
				}

				if (type.value == Type::UNSIGNED_INTEGER && type.size == 2)
				{
					return fits_uint16(*literal_value);
				}

				if (type.value == Type::SIGNED_INTEGER && type.size == 2)
				{
					return fits_int16(*literal_value);
				}

				if (type.value == Type::UNSIGNED_INTEGER && type.size == 4)
				{
					return fits_uint32(*literal_value);
				}

				if (type.value == Type::SIGNED_INTEGER && type.size == 4)
				{
					return fits_int32(*literal_value);
				}

				if (type.value == Type::UNSIGNED_INTEGER && type.size == 8)
				{
					return fits_uint64(*literal_value);
				}

				if (type.value == Type::SIGNED_INTEGER && type.size == 8)
				{
					return fits_int64(*literal_value);
				}
			}

			// The type will only fit if it is smaller
			// than or equal to the other type.
			// Note that this does allow overflow if the
			// types are of equal size but one is signed
			// and the other is unsigned.
			// TODO: think about whether this should
			// throw a warning or not.

			if (byte_size() > type.byte_size())
			{
				return false;
			}

			return true;
		}

		// If we're dealing with a user defined class, we can
		// only fit if the other type is from the same class.

		if (value == Type::USER_DEFINED_CLASS)
		{
			if (type.value != Type::USER_DEFINED_CLASS)
			{
				return false;
			}

			if (class_name != type.class_name)
			{
				return false;
			}

			return true;
		}

		// If we're dealing with an init list, we can only
		// fit if the other type is a user defined class
		// and the type list matches the class field list,
		// or if the other type is an array and the items on
		// the init list all fit the array type.

		if (value == Type::INIT_LIST)
		{
			if (type.value == Type::USER_DEFINED_CLASS)
			{
				// The type list must have the same
				// number of elements as the class
				// field list.

				if (fields.size() > type.fields.size())
					return false;

				// Recursively check if each field fits.

				for (size_t i = 0; i < fields.size(); i++)
				{
					if (!fields[i].fits(type.fields[i]))
						return false;
				}

				return true;
			}

			if (type.is_array())
			{
				// Get the array type.

				Type array_item_type = type;
				array_item_type.array_sizes.pop_back();

				// If the number of items on the init
				// list is larger than the number of
				// elements on the array, it will not
				// fit.

				if (fields.size() > type.array_sizes.back())
					return false;

				// Recursively check if each item fits.

				for (size_t i = 0; i < fields.size(); i++)
				{
					if (!fields[i].fits(array_item_type))
						return false;
				}

				return true;
			}
		}

		// In all other cases, the type does not fit.

		return false;
	}

	/**
	 * @brief Converts a type to a string.
	 * Used for displaying error messages regarding types.
	 * @returns A Tea string representation of the type.
	 */
	std::string
	to_str() const
	{
		std::string s;

		// Add the base type name to the string.

		switch (value)
		{
		default:
		case Type::UNDEFINED:
			s += "undefined";
			break;

		case Type::SIGNED_INTEGER:
			s += "int" + std::to_string(size * 8);
			break;

		case Type::UNSIGNED_INTEGER:
			s += "uint" + std::to_string(size * 8);
			break;

		case Type::USER_DEFINED_CLASS:
			s += class_name;
			break;

		case Type::INIT_LIST:
			if (fields.size() == 0)
			{
				s += "{}";
				break;
			}

			s += "{ ";

			for (size_t i = 0; i < fields.size(); i++)
			{
				s += fields[i].to_str();
				if (i != fields.size() - 1)
					s += ", ";
			}

			s += " }";

			break;
		}

		// Add the array sizes to the string.

		for (size_t i = 0; i < array_sizes.size(); i++)
		{
			if (array_sizes[i] == 0)
			{
				s += '*';
			}
			else
			{
				s += '[' + std::to_string(array_sizes[i]) + ']';
			}
		}

		// If the type is a literal, add the literal value.

		if (is_literal)
		{
			s += " (";
			s += *literal_value;
			s += ")";
		}

		return s;
	}

	/**
	 * @brief Converts a type to a debug type.
	 * Used for generating debugger symbols.
	 * @returns A debugger symbol type.
	 */
	DebuggerSymbolType
	to_debug_type()
		const
	{
		if (pointer_depth() > 0)
			return DebuggerSymbolType::POINTER;

		if (value == Type::UNSIGNED_INTEGER)
		{
			switch (size)
			{
			case 1:
				return DebuggerSymbolType::U8;
			case 2:
				return DebuggerSymbolType::U16;
			case 4:
				return DebuggerSymbolType::U32;
			case 8:
				return DebuggerSymbolType::U64;
			default:
				return DebuggerSymbolType::UNDEFINED;
			}
		}
		else if (value == Type::SIGNED_INTEGER)
		{
			switch (size)
			{
			case 1:
				return DebuggerSymbolType::I8;
			case 2:
				return DebuggerSymbolType::I16;
			case 4:
				return DebuggerSymbolType::I32;
			case 8:
				return DebuggerSymbolType::I64;
			default:
				return DebuggerSymbolType::UNDEFINED;
			}
		}
		else if (value == Type::USER_DEFINED_CLASS)
		{
			return DebuggerSymbolType::USER_DEFINED_CLASS;
		}

		return DebuggerSymbolType::UNDEFINED;
	}
};

#endif