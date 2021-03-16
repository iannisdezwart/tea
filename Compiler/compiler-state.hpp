#ifndef TEA_COMPILER_STATE_HEADER
#define TEA_COMPILER_STATE_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "../Assembler/assembler.hpp"
#include "../VM/cpu.hpp"

using namespace std;

enum class IdentifierKind {
	UNDEFINED,
	GLOBAL,
	FUNCTION,
	PARAMETER,
	LOCAL
};

class Type {
	public:
		enum Value : uint8_t {
			UNDEFINED,
			UNSIGNED_INTEGER,
			SIGNED_INTEGER,
			USER_DEFINED_CLASS
		};

		Type() : value(UNDEFINED) {}
		Type(enum Value value, size_t size) : value(value), size(size) {}

		// Allow switch comparisons

		operator Value() const { return value; }

		// Don't allow conversion to boolean

		explicit operator bool() = delete;

		constexpr bool operator==(const Type& other) const
		{
			return value == other.value && size == other.size;
		}

		constexpr bool operator!=(const Type& other) const
		{
			return value != other.value || size != other.size;
		}

		constexpr size_t byte_size()
		{
			return size;
		}

		static Type from_string(string str)
		{
			if (str == "uint8" || str == "char")
				return Type(Type::UNSIGNED_INTEGER, 1);

			if (str == "int8")
				return Type(Type::SIGNED_INTEGER, 1);

			if (str == "uint16")
				return Type(Type::UNSIGNED_INTEGER, 2);

			if (str == "int16")
				return Type(Type::SIGNED_INTEGER, 2);

			if (str == "uint32")
				return Type(Type::UNSIGNED_INTEGER, 4);

			if (str == "int32" || str == "int")
				return Type(Type::SIGNED_INTEGER, 4);

			if (str == "uint64")
				return Type(Type::UNSIGNED_INTEGER, 8);

			if (str == "int64")
				return Type(Type::SIGNED_INTEGER, 8);

			err("Wasn't able to convert \"%s\" to a Type", str.c_str());
		}

		bool fits_in_register()
		{
			return (value == Type::SIGNED_INTEGER || value == Type::UNSIGNED_INTEGER)
				&& size <= 8;
		}

	private:
		enum Value value;
		size_t size;
};

struct Variable {
	size_t offset;
	Type type;

	Variable() {}
	Variable(Type& type, size_t offset) : type(type), offset(offset) {}
};

struct Function {
	Type return_type;
	vector<Type> parameter_types;

	Function() {}
	Function(Type& return_type) : return_type(return_type) {}
};

class CompilerState {
	public:
		map<string, Variable> globals;
		unordered_map<string, Function> functions;

		map<string, Variable> parameters;
		uint64_t parameters_size = 0;

		map<string, Variable> locals;

		enum IdentifierKind get_identifier_kind(string id_name)
		{
			if (locals.count(id_name)) return IdentifierKind::LOCAL;
			if (parameters.count(id_name)) return IdentifierKind::PARAMETER;
			if (functions.count(id_name)) return IdentifierKind::FUNCTION;
			if (globals.count(id_name)) return IdentifierKind::GLOBAL;
			return IdentifierKind::UNDEFINED;
		}

		bool add_global(string id_name, Type id_type, size_t offset)
		{
			if (globals.count(id_name) || functions.count(id_name)) return false;

			globals[id_name] = Variable(id_type, offset);
			return true;
		}

		bool add_function(string function_name, Function function_type)
		{
			if (functions.count(function_name) || globals.count(function_name))
				return false;

			functions[function_name] = function_type;
			return true;
		}

		bool add_parameter(string param_name, Type param_type, size_t offset)
		{
			if (parameters.count(param_name)) return false;

			parameters[param_name] = Variable(param_type, offset);
			parameters_size += offset;
			return true;
		}

		bool add_local(string local_name, Type local_type, size_t offset)
		{
			if (locals.count(local_name)) return false;

			locals[local_name] = Variable(local_type, offset);
			return true;
		}

		void end_function_scope()
		{
			locals.clear();
			parameters.clear();
			parameters_size = 0;
		}
};

#endif