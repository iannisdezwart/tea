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
			USER_DEFINED_CLASS,
			INIT_LIST
		};

		vector<size_t> array_sizes;

		string class_name;
		vector<Type> fields;

		bool is_literal = false;
		string *literal_value;

		Type() : value(UNDEFINED) {}

		Type(enum Value value, size_t size) : value(value), size(size) {}

		Type(enum Value value, size_t size, const vector<size_t>& array_sizes)
			: value(value), size(size), array_sizes(array_sizes) {}

		size_t pointer_depth() const
		{
			return array_sizes.size();
		}

		// Allow switch comparisons

		operator Value() const { return value; }

		// Don't allow conversion to boolean

		explicit operator bool() = delete;

		constexpr bool operator==(const Type& other) const
		{
			return value == other.value && size == other.size
				&& pointer_depth() == other.pointer_depth();
		}

		constexpr bool operator!=(const Type& other) const
		{
			return value != other.value || size != other.size
				|| pointer_depth() != other.pointer_depth();
		}

		constexpr bool operator==(Type::Value other_value) const
		{
			return value == other_value;
		}

		constexpr bool operator!=(Type::Value other_value) const
		{
			return value != other_value;
		}

		size_t byte_size(size_t deref_dep = 0) const
		{
			return (pointer_depth() - deref_dep > 0) ? 8 : size;
		}

		size_t pointed_byte_size() const
		{
			return size;
		}

		Type pointed_type() const
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

		size_t storage_size() const
		{
			if (pointer_depth() == 0) return byte_size();

			size_t n_members = 1;
			size_t dim = 0;

			for (size_t i = pointer_depth(); i != 0; i--) {
				if (array_sizes[i - 1] == 0) break;
				n_members *= array_sizes[i - 1];
				dim++;
			}

			return n_members * byte_size(dim);
		}

		bool is_array() const
		{
			if (array_sizes.size() == 0) return false;
			return array_sizes.back() != 0;
		}

		static Type from_string(string str, const vector<size_t>& array_sizes)
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

		bool fits_in_register()
		{
			return (value == Type::SIGNED_INTEGER || value == Type::UNSIGNED_INTEGER)
				&& size <= 8;
		}

		bool fits(const Type& type) const
		{
			if (value == Type::UNSIGNED_INTEGER || value == Type::SIGNED_INTEGER) {
				if (type.value != Type::UNSIGNED_INTEGER &&
					type.value != Type::SIGNED_INTEGER)
				{
					return false;
				}

				if (is_literal) {
					if (type.value == Type::UNSIGNED_INTEGER && type.size == 1) {
						return fits_uint8(*literal_value);
					}

					if (type.value == Type::SIGNED_INTEGER && type.size == 1) {
						return fits_int8(*literal_value);
					}

					if (type.value == Type::UNSIGNED_INTEGER && type.size == 2) {
						return fits_uint16(*literal_value);
					}

					if (type.value == Type::SIGNED_INTEGER && type.size == 2) {
						return fits_int16(*literal_value);
					}

					if (type.value == Type::UNSIGNED_INTEGER && type.size == 4) {
						return fits_uint32(*literal_value);
					}

					if (type.value == Type::SIGNED_INTEGER && type.size == 4) {
						return fits_int32(*literal_value);
					}

					if (type.value == Type::UNSIGNED_INTEGER && type.size == 8) {
						return fits_uint64(*literal_value);
					}

					if (type.value == Type::SIGNED_INTEGER && type.size == 8) {
						return fits_int64(*literal_value);
					}
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

			if (value == Type::INIT_LIST) {
				if (type.value == Type::USER_DEFINED_CLASS) {
					if (fields.size() > type.fields.size()) return false;

					for (size_t i = 0; i < fields.size(); i++) {
						if (!fields[i].fits(type.fields[i])) return false;
					}

					return true;
				}

				if (type.is_array()) {
					Type array_item_type = type;
					array_item_type.array_sizes.pop_back();

					if (fields.size() > type.array_sizes.back()) return false;

					for (size_t i = 0; i < fields.size(); i++) {
						if (!fields[i].fits(array_item_type)) return false;
					}

					return true;
				}
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

				case Type::INIT_LIST:
					if (fields.size() == 0) {
						s += "{}";
						break;
					}

					s += "{ ";

					for (size_t i = 0; i < fields.size(); i++) {
						s += fields[i].to_str();
						if (i != fields.size() - 1) s += ", ";
					}

					s += " }";

					break;
			}

			if (pointer_depth()) {
				for (size_t i = 0; i < array_sizes.size(); i++) {
					if (array_sizes[i] == 0) {
						s += '*';
					} else {
						s += '[' + to_string(array_sizes[i]) + ']';
					}
				}
			}

			if (is_literal) {
				s += " (";
				s += *literal_value;
				s += ")";
			}

			return s;
		}

		enum DebuggerSymbolTypes to_debug_type()
		{
			if (pointer_depth() > 0) return DebuggerSymbolTypes::POINTER;

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
	vector<Function> methods;

	Class() {}
	Class(size_t byte_size): byte_size(byte_size) {}

	void add_field(const string& field_name, const Type& field_type)
	{
		fields.push_back(Identifier(field_name, field_type));
	}

	bool has_field(const string& field_name) const
	{
		for (const Identifier& field : fields) {
			if (field.name == field_name) return true;
		}

		return false;
	}

	const Type& get_field_type(const string& field_name) const
	{
		for (const Identifier& field : fields) {
			if (field.name == field_name) return field.type;
		}

		err("Class field \"%s\" not found", field_name.c_str());
	}

	void add_method(const string& method_name, const Function& method)
	{
		methods.push_back(method);
	}

	bool has_method(const string& method_name) const
	{
		for (const Function& method : methods) {
			if (method.id.name == method_name) return true;
		}

		return false;
	}

	const Function& get_method(const string& method_name) const
	{
		for (const Function& method : methods) {
			if (method.id.name == method_name) return method;
		}

		err("Class method \"%s\" not found", method_name.c_str());
	}
};

struct LocationData {
	IdentifierKind id_kind;
	int64_t offset;
	uint64_t var_size;

	LocationData(IdentifierKind id_kind, int64_t offset, uint64_t var_size)
		: id_kind(id_kind), offset(offset), var_size(var_size) {}

	bool is_at_stack_top()
	{
		return id_kind == IdentifierKind::GLOBAL;
	}

	bool is_at_frame_top()
	{
		return id_kind == IdentifierKind::LOCAL || id_kind == IdentifierKind::PARAMETER;
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
			globals_size += global_type.storage_size();

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
			locals_size += local_type.storage_size();
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