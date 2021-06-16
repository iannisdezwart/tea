#ifndef TEA_COMPILER_STATE_HEADER
#define TEA_COMPILER_STATE_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "../Assembler/assembler.hpp"
#include "../VM/cpu.hpp"
#include "debugger-symbols.hpp"

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
		string class_name;
		vector<Type> fields;

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

		constexpr bool operator==(Type::Value other_value) const
		{
			return value == other_value;
		}

		constexpr bool operator!=(Type::Value other_value) const
		{
			return value != other_value;
		}

		constexpr size_t byte_size() const
		{
			return (pointer_depth > 0) ? 8 : size;
		}

		constexpr size_t pointed_byte_size() const
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

		bool fits(const Type& type)
		{
			if (value == Type::UNSIGNED_INTEGER || value == Type::SIGNED_INTEGER) {
				if (type.value != Type::UNSIGNED_INTEGER &&
					type.value != Type::SIGNED_INTEGER)
				{
					return false;
				}

				if (byte_size() > type.byte_size()) {
					return false;
				}

				return true;
			}

			if (value == Type::USER_DEFINED_CLASS) {
				if (type.value != Type::USER_DEFINED_CLASS) {
					return false;
				}

				if (class_name != type.class_name) {
					return false;
				}

				return true;
			}

			return false;
		}

		string to_str() const
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
					s += class_name;
					break;
			}

			if (pointer_depth) {
				s += ' ';
				s += string(pointer_depth, '*');
			}

			return s;
		}

		enum DebuggerSymbolTypes to_debug_type()
		{
			if (pointer_depth > 0) return DebuggerSymbolTypes::POINTER;

			if (value == Type::UNSIGNED_INTEGER) {
				switch (size) {
					case 1: return DebuggerSymbolTypes::U8;
					case 2: return DebuggerSymbolTypes::U16;
					case 4: return DebuggerSymbolTypes::U32;
					case 8: return DebuggerSymbolTypes::U64;
					default: return DebuggerSymbolTypes::UNDEFINED;
				}
			} else if (value == Type::SIGNED_INTEGER) {
				switch (size) {
					case 1: return DebuggerSymbolTypes::I8;
					case 2: return DebuggerSymbolTypes::I16;
					case 4: return DebuggerSymbolTypes::I32;
					case 8: return DebuggerSymbolTypes::I64;
					default: return DebuggerSymbolTypes::UNDEFINED;
				}
			} else if (value == Type::USER_DEFINED_CLASS) {
				return DebuggerSymbolTypes::USER_DEFINED_CLASS;
			}

			return DebuggerSymbolTypes::UNDEFINED;
		}

	private:
		enum Value value;
		size_t size;
};

struct Identifier
{
	string name;
	Type type;

	Identifier() {}
	Identifier(const string& name, const Type& type) : name(name), type(type) {}
};

struct Variable {
	size_t offset;
	Identifier id;

	Variable() {}
	Variable(const string& name, Type& type, size_t offset)
		: id(name, type), offset(offset) {}
};

struct Function {
	Identifier id;
	vector<Identifier> parameters;

	Function() {}
	Function(const string& fn_name, Type& return_type)
		: id(fn_name, return_type) {}

	void add_parameter(const string& param_name, const Type& param_type)
	{
		parameters.push_back(Identifier(param_name, param_type));
	}
};

struct Class {
	size_t byte_size;
	vector<Identifier> fields;

	Class() {}
	Class(size_t byte_size): byte_size(byte_size) {}

	void add_field(const string& field_name, const Type& field_type)
	{
		fields.push_back(Identifier(field_name, field_type));
	}

	bool contains_field(const string& field_name) const
	{
		for (const Identifier& field : fields) {
			if (field.name == field_name) return true;
		}

		return false;
	}
};

class CompilerState {
	public:
		unordered_map<string, Function> functions;
		string current_function_name;

		unordered_map<string, Class> classes;

		unordered_map<string, Variable> globals;
		uint64_t globals_size = 0;

		unordered_map<string, Variable> parameters;
		vector<string> parameter_names_in_order;
		uint64_t parameters_size = 0;

		unordered_map<string, Variable> locals;
		vector<string> local_names_in_order;
		uint64_t locals_size = 0;

		size_t scope_depth = 0;
		bool root_of_operation_tree = true;

		uint64_t label_id = 0;

		const bool debug;
		DebuggerSymbols debugger_symbols;

		CompilerState(bool debug) : debug(debug) {}

		enum IdentifierKind get_identifier_kind(string id_name)
		{
			if (locals.count(id_name)) return IdentifierKind::LOCAL;
			if (parameters.count(id_name)) return IdentifierKind::PARAMETER;
			if (functions.count(id_name)) return IdentifierKind::FUNCTION;
			if (globals.count(id_name)) return IdentifierKind::GLOBAL;
			return IdentifierKind::UNDEFINED;
		}

		bool add_class(string class_name, Class cl)
		{
			if (classes.count(class_name)) return false;

			classes[class_name] = cl;

			// Add the class to the debugger symbols

			if (debug) {
				// Todo: create
			}

			return true;
		}

		bool add_global(string global_name, Type global_type)
		{
			if (globals.count(global_name) || functions.count(global_name))
				return false;

			globals[global_name] = Variable(global_name, global_type, globals_size);
			globals_size += global_type.byte_size();

			// Add the global to the debugger symbols

			if (debug) {
				debugger_symbols.add_global(DebuggerSymbol(global_name, global_type.to_debug_type()));
			}

			return true;
		}

		bool add_function(string function_name, Function function_type)
		{
			if (functions.count(function_name) || globals.count(function_name))
				return false;

			current_function_name = function_name;
			functions[function_name] = function_type;
			return true;
		}

		bool add_parameter(string param_name, Type param_type)
		{
			if (parameters.count(param_name)) return false;

			parameters[param_name] = Variable(param_name, param_type, parameters_size);
			parameter_names_in_order.push_back(param_name);
			parameters_size += param_type.byte_size();
			return true;
		}

		bool add_local(string local_name, Type local_type)
		{
			if (locals.count(local_name)) return false;

			locals[local_name] = Variable(local_name, local_type, locals_size);
			local_names_in_order.push_back(local_name);
			locals_size += local_type.byte_size();
			return true;
		}

		void end_function_scope()
		{
			// Add the function to the debugger symbols

			if (debug) {
				DebuggerFunction fn_symbols;

				// Add params

				for (const string& param_name : parameter_names_in_order) {
					DebuggerSymbolTypes fn_param_type = parameters[param_name].id.type.to_debug_type();
					fn_symbols.params.push_back(DebuggerSymbol(param_name, fn_param_type));
				}

				// Add locals

				for (const string& local_name : local_names_in_order) {
					DebuggerSymbolTypes local_type = locals[local_name].id.type.to_debug_type();
					fn_symbols.locals.push_back(DebuggerSymbol(local_name, local_type));
				}

				debugger_symbols.add_function(current_function_name, fn_symbols);
			}

			locals.clear();
			local_names_in_order.clear();
			locals_size = 0;

			parameters.clear();
			parameter_names_in_order.clear();
			parameters_size = 0;
		}

		Type get_type_of_identifier(string id_name)
		{
			IdentifierKind id_kind = get_identifier_kind(id_name);

			switch (id_kind) {
				case IdentifierKind::LOCAL:
					return locals[id_name].id.type;

				case IdentifierKind::PARAMETER:
					return parameters[id_name].id.type;

				case IdentifierKind::GLOBAL:
					return globals[id_name].id.type;

				default:
					return Type();
			}
		}

		string generate_label(string type)
		{
			string label = "compiler-generated-label-";
			label += to_string(label_id++);
			label += "-for-";
			label += type;
			return label;
		}
};

#endif