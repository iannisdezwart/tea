#ifndef TEA_TYPE_HEADER
#define TEA_TYPE_HEADER

#include "Compiler/util.hpp"
#include "Compiler/debugger-symbols.hpp"

// clang-format off
enum BuiltinType : uint
{
	UNDEFINED,
	V0, U8, I8, U16, I16, U32, I32, U64, I64, F32, F64,
	BUILTIN_TYPE_END,
};

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
static std::vector<std::vector<uint>> type_array_sizes = { {} };
// clang-format on

/**
 * Class that represents a data type of a variable.
 *
 * This class has lots of handy methods to get additional information about
 * the data type it corresponds.
 */
struct Type
{
	uint value;
	uint size;
	uint array_sizes_idx;

	// If the subtype is a user-defined class,
	// this field will hold the id of the class.
	uint32_t class_id {};

	/**
	 * @brief Default constructor.
	 * Sets the type to undefined, so this has to be updated later.
	 * It's here to support putting Type objects in a vector etc.
	 */
	Type()
		: value(UNDEFINED) {}

	Type(uint value, uint size, uint array_sizes_idx = 0)
		: value(value), size(size), array_sizes_idx(array_sizes_idx) {}

	/**
	 * Returns the pointer depth of this type.
	 *
	 * Examples:
	 *   * u64* -> 1
	 *   * u64[3][4][5] -> 3
	 *   * u64[3]*[5] -> 3
	 */
	uint
	pointer_depth() const
	{
		if (array_sizes_idx == 0)
			return 0;
		return type_array_sizes[array_sizes_idx].size();
	}

	/**
	 * @brief Compares two types with each other for equality.
	 * @param other The other type.
	 * @returns True if the types have the same subtype
	 * and array dimensions, false otherwise.
	 */
	constexpr bool
	operator==(const Type &other) const
	{
		return value == other.value && pointer_depth() == other.pointer_depth();
	}

	/**
	 * @brief Compares two types with each other for inequality.
	 * @param other The other type.
	 * @returns True if the types don't have the same subtype
	 * and array dimensions, false otherwise.
	 */
	constexpr bool
	operator!=(const Type &other) const
	{
		return value != other.value || pointer_depth() != other.pointer_depth();
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
		if (array_sizes_idx == 0)
		{
			err("Compiler error: tried dereferencing non-pointer Type %s",
				to_str().c_str());
		}

		uint new_arr_sz_idx          = type_array_sizes.size();
		std::vector<uint> new_arr_sz = type_array_sizes[array_sizes_idx];
		new_arr_sz.erase(new_arr_sz.begin());
		type_array_sizes.push_back(new_arr_sz);

		return Type(value, size, new_arr_sz_idx);
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

		const std::vector<uint> &array_sz = type_array_sizes[array_sizes_idx];
		for (size_t i = pointer_depth(); i != 0; i--)
		{
			if (array_sz[i - 1] == 0)
				break;
			n_members *= array_sz[i - 1];
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
		if (array_sizes_idx == 0)
			return false;
		const std::vector<uint> &array_sz = type_array_sizes[array_sizes_idx];
		if (array_sz.size() == 0)
			return false;
		return array_sz.back() != 0;
	}

	/**
	 * @returns A boolean indicating whether this type is a class.
	 * A pointer to a class is not a class.
	 */
	bool
	is_class() const
	{
		if (value < BUILTIN_TYPE_END)
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
		switch (value)
		{
		case V0:
		case U8:
		case I8:
		case U16:
		case I16:
		case U32:
		case I32:
		case U64:
		case I64:
			return true;
		default:
			return false;
		}
	}

	bool
	is_float() const
	{
		return value == F32 || value == F64;
	}

	/**
	 * @brief Converts a string to a type.
	 * Only the standard primitive Tea types are supported:
	 * u8, i8, u16, u32, i32, u64, i64, f32, f64, void.
	 * @param str The string to convert.
	 * @param array_sizes_idx
	 * @returns The type parsed from the string.
	 */
	static Type
	from_string(std::string str, uint array_sizes_idx)
	{
		if (str == "u8")
			return Type(U8, 1, array_sizes_idx);

		if (str == "i8")
			return Type(I8, 1, array_sizes_idx);

		if (str == "u16")
			return Type(U16, 2, array_sizes_idx);

		if (str == "i16")
			return Type(I16, 2, array_sizes_idx);

		if (str == "u32")
			return Type(U32, 4, array_sizes_idx);

		if (str == "i32")
			return Type(I32, 4, array_sizes_idx);

		if (str == "u64")
			return Type(U64, 8, array_sizes_idx);

		if (str == "i64")
			return Type(I64, 8, array_sizes_idx);

		if (str == "f32")
			return Type(F32, 4, array_sizes_idx);

		if (str == "f64")
			return Type(F64, 8, array_sizes_idx);

		if (str == "v0")
			return Type(V0, 0, array_sizes_idx);

		err("Wasn't able to convert \"%s\" to a Type", str.c_str());
	}

	/**
	 * @returns A boolean indicating whether the type is a primitive type.
	 */
	bool
	is_primitive() const
	{
		switch (value)
		{
		case V0:
		case U8:
		case I8:
		case U16:
		case I16:
		case U32:
		case I32:
		case U64:
		case I64:
		case F32:
		case F64:
			return true;
		default:
			return false;
		}
	}

	/**
	 * @returns A boolean indicating whether the type
	 * fits in a register. A type only fits in a register
	 * if it is a primitive type.
	 */
	bool
	fits_in_register()
	{
		return is_primitive() && size <= 8;
	}

	enum struct Fits
	{
		YES,
		NO,
		FLT_32_TO_INT_CAST_NEEDED,
		FLT_64_TO_INT_CAST_NEEDED,
		INT_TO_FLT_32_CAST_NEEDED,
		INT_TO_FLT_64_CAST_NEEDED,
	};

	/**
	 * @returns A value indicating whether the type fits within
	 * another type. This means that this type can be promoted
	 * to the other type.
	 * @param type The other type.
	 */
	Fits
	fits(const Type &type) const
	{
		// Primitives

		if (is_primitive())
		{
			// If the other type is not a primitive,
			// it will definitely not fit.

			if (!type.is_primitive())
			{
				return Fits::NO;
			}

			// Type won't fit if it is larger than the other type.

			if (byte_size() > type.byte_size())
			{
				return Fits::NO;
			}

			// Floating point types can only fit in other floating point types.

			if ((value == F32 || value == F64) && type.is_integer())
			{
				if (size == 4)
					return Fits::FLT_32_TO_INT_CAST_NEEDED;
				else
					return Fits::FLT_64_TO_INT_CAST_NEEDED;
			}

			// Integer types can only fit in other integer types.

			if (is_integer() && (type.value == F32 || type.value == F64))
			{
				if (type.size == 4)
					return Fits::INT_TO_FLT_32_CAST_NEEDED;
				else
					return Fits::INT_TO_FLT_64_CAST_NEEDED;
			}

			return Fits::YES;
		}

		// If we're dealing with a user defined class, we can
		// only fit if the other type is from the same class.

		if (value >= BUILTIN_TYPE_END)
		{
			if (value != type.value)
			{
				return Fits::NO;
			}

			return Fits::YES;
		}

		// In all other cases, the type does not fit.

		return Fits::NO;
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
		case UNDEFINED:
			s += "undefined";
			break;

		case V0:
			s += "v0";
			break;

		case U8:
			s += "u8";
			break;

		case I8:
			s += "i8";
			break;

		case U16:
			s += "u16";
			break;

		case I16:
			s += "i16";
			break;

		case U32:
			s += "u32";
			break;

		case I32:
			s += "i32";
			break;

		case U64:
			s += "u64";
			break;

		case I64:
			s += "i64";
			break;

		case F32:
			s += "f32";
			break;

		case F64:
			s += "f64";
			break;

		default:
			s += "class(" + std::to_string(value) + ")";
			break;
		}

		if (array_sizes_idx == 0)
			return s;

		// Add the array sizes to the string.

		const std::vector<uint> &array_sz = type_array_sizes[array_sizes_idx];
		for (size_t i = 0; i < array_sz.size(); i++)
		{
			if (array_sz[i] == 0)
			{
				s += '*';
			}
			else
			{
				s += '[' + std::to_string(array_sz[i]) + ']';
			}
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

		switch (value)
		{
		case V0:
			return DebuggerSymbolType::UNDEFINED;
		case U8:
			return DebuggerSymbolType::U8;
		case I8:
			return DebuggerSymbolType::I8;
		case U16:
			return DebuggerSymbolType::U16;
		case I16:
			return DebuggerSymbolType::I16;
		case U32:
			return DebuggerSymbolType::U32;
		case I32:
			return DebuggerSymbolType::I32;
		case U64:
			return DebuggerSymbolType::U64;
		case I64:
			return DebuggerSymbolType::I64;
		case F32:
			return DebuggerSymbolType::F32;
		case F64:
			return DebuggerSymbolType::F64;
		default:
			return DebuggerSymbolType::USER_DEFINED_CLASS;
		}
	}
};

constexpr int TYPE_SIZE = sizeof(Type);

#endif