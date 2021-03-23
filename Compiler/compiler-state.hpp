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

		size_t pointer_depth;

		Type() : value(UNDEFINED) {}
		Type(enum Value value, size_t size, size_t pointer_depth = 0)
			: value(value), size(size), pointer_depth(pointer_depth) {}

		// Allow switch comparisons

		operator Value() const { return value; }

		// Don't allow conversion to boolean

		explicit operator bool() = delete;

		constexpr bool operator==(const Type& other) const
		{
			return value == other.value && size == other.size
				&& pointer_depth == other.pointer_depth;
		}

		constexpr bool operator!=(const Type& other) const
		{
			return value != other.value || size != other.size
				|| pointer_depth != other.pointer_depth;
		}

		constexpr size_t byte_size()
		{
			return size;
		}

		static Type from_string(string str, size_t pointer_depth = 0)
		{
			if (str == "u8")
				return Type(Type::UNSIGNED_INTEGER, 1, pointer_depth);

			if (str == "i8")
				return Type(Type::SIGNED_INTEGER, 1, pointer_depth);

			if (str == "u16")
				return Type(Type::UNSIGNED_INTEGER, 2, pointer_depth);

			if (str == "i16")
				return Type(Type::SIGNED_INTEGER, 2, pointer_depth);

			if (str == "u32")
				return Type(Type::UNSIGNED_INTEGER, 4, pointer_depth);

			if (str == "i32")
				return Type(Type::SIGNED_INTEGER, 4, pointer_depth);

			if (str == "u64")
				return Type(Type::UNSIGNED_INTEGER, 8, pointer_depth);

			if (str == "i64")
				return Type(Type::SIGNED_INTEGER, 8, pointer_depth);

			if (str == "void")
				return Type(Type::UNSIGNED_INTEGER, 0, pointer_depth);

			err("Wasn't able to convert \"%s\" to a Type", str.c_str());
		}

		bool fits_in_register()
		{
			return (value == Type::SIGNED_INTEGER || value == Type::UNSIGNED_INTEGER)
				&& size <= 8;
		}

		string to_str()
		{
			string s;

			switch (value) {
				default:
				case Type::UNDEFINED:
					s += "undefined";
					break;

				case Type::SIGNED_INTEGER:
					s += "int" + to_string(size * 8);
					break;

				case Type::UNSIGNED_INTEGER:
					s += "uint" + to_string(size * 8);
					break;

				case Type::USER_DEFINED_CLASS:
					s += "user_defined_class";
					break;
			}

			if (pointer_depth) {
				s += ' ';
				s += string(pointer_depth, '*');
			}

			return s;
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
		unordered_map<string, Function> functions;

		unordered_map<string, Variable> globals;
		uint64_t globals_size = 0;

		unordered_map<string, Variable> parameters;
		uint64_t parameters_size = 0;

		unordered_map<string, Variable> locals;
		uint64_t locals_size = 0;

		size_t scope_depth = 0;
		bool root_of_operation_tree = true;

		enum IdentifierKind get_identifier_kind(string id_name)
		{
			if (locals.count(id_name)) return IdentifierKind::LOCAL;
			if (parameters.count(id_name)) return IdentifierKind::PARAMETER;
			if (functions.count(id_name)) return IdentifierKind::FUNCTION;
			if (globals.count(id_name)) return IdentifierKind::GLOBAL;
			return IdentifierKind::UNDEFINED;
		}

		bool add_global(string global_name, Type global_type)
		{
			if (globals.count(global_name) || functions.count(global_name))
				return false;

			globals[global_name] = Variable(global_type, globals_size);
			globals_size += global_type.byte_size();
			return true;
		}

		bool add_function(string function_name, Function function_type)
		{
			if (functions.count(function_name) || globals.count(function_name))
				return false;

			functions[function_name] = function_type;
			return true;
		}

		bool add_parameter(string param_name, Type param_type)
		{
			if (parameters.count(param_name)) return false;

			parameters[param_name] = Variable(param_type, parameters_size);
			parameters_size += param_type.byte_size();
			return true;
		}

		bool add_local(string local_name, Type local_type)
		{
			if (locals.count(local_name)) return false;

			locals[local_name] = Variable(local_type, locals_size);
			locals_size += local_type.byte_size();
			return true;
		}

		void end_function_scope()
		{
			locals.clear();
			locals_size = 0;

			parameters.clear();
			parameters_size = 0;
		}

		Type get_type_of_identifier(string id_name)
		{
			IdentifierKind id_kind = get_identifier_kind(id_name);

			switch (id_kind) {
				case IdentifierKind::LOCAL:
					return locals[id_name].type;

				case IdentifierKind::PARAMETER:
					return parameters[id_name].type;

				case IdentifierKind::GLOBAL:
					return globals[id_name].type;

				default:
					return Type();
			}
		}
};

#endif