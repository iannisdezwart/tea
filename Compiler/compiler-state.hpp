#ifndef TEA_COMPILER_STATE_HEADER
#define TEA_COMPILER_STATE_HEADER

#include <bits/stdc++.h>

#include "util.hpp"
#include "../Assembler/assembler.hpp"
#include "../VM/cpu.hpp"
#include "debugger-symbols.hpp"
#include "type.hpp"

/**
 * @brief Enum for the different types of identifiers:
 * globals, functions, parameters and locals.
 */
enum class IdentifierKind {
	UNDEFINED,
	GLOBAL,
	FUNCTION,
	PARAMETER,
	LOCAL
};

/**
 * @brief Structure for the identifiers.
 * Holds the name and type.
 */
struct Identifier
{
	// The name of this identifier.
	std::string name;

	// The data type of this identifier.
	Type type;

	Identifier() {}
	Identifier(const std::string& name, const Type& type) : name(name), type(type) {}
};

/**
 * @brief Structure for a variable type.
 * Holds the name, type and offset.
 * Depending on whether the variable is a global, local or parameter,
 * the offset is either relative to the top of the stack,
 * or the start of the current stack frame.
 */
struct Variable {
	// The offset of this variable.
	// Depending on whether the variable is a global, local or parameter,
	// the offset is either relative to the top of the stack,
	// or the start of the current stack frame.
	size_t offset;

	// Holds the name and type of this variable.
	Identifier id;

	Variable() {}
	Variable(const std::string& name, Type& type, size_t offset)
		: id(name, type), offset(offset) {}
};

/**
 * @brief Structure for a function type.
 * Contains the name and return type, as well as the parameter identifiers.
 */
struct Function {
	// Holds the name and return type of this function.
	Identifier id;

	// Holds the identifiers of the parameters of this function.
	std::vector<Identifier> parameters;

	Function() {}
	Function(const std::string& fn_name, Type& return_type)
		: id(fn_name, return_type) {}

	/**
	 * @brief Adds a parameter to this function.
	 * @param param_name The name of the parameter.
	 * @param param_type The type of the parameter.
	 */
	void add_parameter(const std::string& param_name, const Type& param_type)
	{
		parameters.push_back(Identifier(param_name, param_type));
	}
};

/**
 * @brief Structure for a class type.
 * Holds the byte size of the class,
 * as well as the identifiers and methods of the class.
 * Does not contain the name of the class,
 * since that will be the key to the class in the `compiler_state.classes` map.
 */
struct Class {
	// The byte size of the class.
	size_t byte_size;

	// A list of all fields in the class.
	std::vector<Identifier> fields;

	// A list of all methods in the class.
	std::vector<Function> methods;

	Class() {}
	Class(size_t byte_size): byte_size(byte_size) {}

	/**
	 * @brief Adds a field to this class.
	 * @param field_name The name of the field.
	 * @param field_type The type of the field.
	 */
	void add_field(const std::string& field_name, const Type& field_type)
	{
		fields.push_back(Identifier(field_name, field_type));
	}

	/**
	 * @param field_name The field name to look for.
	 * @returns A boolean indicating whether a field with this name exists.
	 */
	bool has_field(const std::string& field_name) const
	{
		for (const Identifier& field : fields) {
			if (field.name == field_name) return true;
		}

		return false;
	}

	/**
	 * @param field_name The field name to look for.
	 * @returns The type of a field.
	 */
	const Type& get_field_type(const std::string& field_name) const
	{
		for (const Identifier& field : fields) {
			if (field.name == field_name) return field.type;
		}

		err("Class field \"%s\" not found", field_name.c_str());
	}

	/**
	 * @brief Adds a method to this class.
	 * @param method_name The name of the method.
	 * @param method_type The type of the method.
	 */
	void add_method(const std::string& method_name, const Function& method)
	{
		methods.push_back(method);
	}

	/**
	 * @param method_name The method name to look for.
	 * @returns A boolean indicating whether a method with this name exists.
	 */
	bool has_method(const std::string& method_name) const
	{
		for (const Function& method : methods) {
			if (method.id.name == method_name) return true;
		}

		return false;
	}

	/**
	 * @param method_name The method name to look for.
	 * @returns The type of a method.
	 */
	const Function& get_method(const std::string& method_name) const
	{
		for (const Function& method : methods) {
			if (method.id.name == method_name) return method;
		}

		err("Class method \"%s\" not found", method_name.c_str());
	}
};

/**
 * @brief Structure for location information.
 * Holds the kind of identifier (global, function, parameter or local),
 * as well as the offset and byte size of the identifier.
 */
struct LocationData {
	// The kind of identifier (global, function, parameter or local).
	IdentifierKind id_kind;

	// The offset of the identifier.
	// Depending on whether the identifier is a global, local or parameter,
	// the offset is either relative to the top of the stack,
	// or the start of the current stack frame.
	int64_t offset;

	// The byte size of the identifier.
	uint64_t var_size;

	LocationData(IdentifierKind id_kind, int64_t offset, uint64_t var_size)
		: id_kind(id_kind), offset(offset), var_size(var_size) {}

	/**
	 * @returns A boolean indicating whether this location is a global.
	 */
	bool is_at_stack_top()
	{
		return id_kind == IdentifierKind::GLOBAL;
	}

	/**
	 * @returns A boolean indicating whether this location is
	 * a local or a parameter.
	 */
	bool is_at_frame_top()
	{
		return id_kind == IdentifierKind::LOCAL || id_kind == IdentifierKind::PARAMETER;
	}
};

/**
 * @brief Structure for the compiler state.
 * Holds all the information about the current compilation process.
 * This includes all the identifiers, types, and functions in the current
 * compilation context, as well as the current stack frame.
 */
class CompilerState {
	public:
		// A map of all functions in the current compilation context.
		std::unordered_map<std::string, Function> functions;

		// The current function name being compiled.
		std::string current_function_name;


		// A map of all classes in the current compilation context.
		std::unordered_map<std::string, Class> classes;


		// A map of all global variables in the current compilation context.
		std::unordered_map<std::string, Variable> globals;

		// The size of all global variables combined
		// in the current compilation context.
		uint64_t globals_size = 0;


		// A map of all parameters in the current function being compiled.
		std::unordered_map<std::string, Variable> parameters;

		// The names of the parameters in the current
		// function being compiled, in order.
		std::vector<std::string> parameter_names_in_order;

		// The size of all parameters combined in the
		// current function being compiled.
		uint64_t parameters_size = 0;


		// A map of all local variables in the current function being compiled.
		std::unordered_map<std::string, Variable> locals;

		// The names of the local variables in the current
		// function being compiled, in order.
		std::vector<std::string> local_names_in_order;

		// The size of all local variables combined in the
		// current function being compiled.
		uint64_t locals_size = 0;

		// The current scope depth.
		// Might be useful in the future when we implement
		// functions with nested scopes.
		size_t scope_depth = 0;


		// Generator for unique label identifiers.
		// Used for generating unique labels for different
		// code segments. Starts at 0.
		uint64_t label_id = 0;


		// Whether debug symbols should be generated.
		const bool debug;

		// The debugger symbols.
		DebuggerSymbols debugger_symbols;


		/**
		 * @brief Constructs a new Compiler State object.
		 * @param debug Whether debug symbols should be generated.
		 */
		CompilerState(bool debug) : debug(debug) {}

		/**
		 * @param id_name The identifier to look for.
		 * @returns The identifier kind of a given identifier.
		 *
		 * The identifier is searched in this order:
		 * * Local variables
		 * * Parameters
		 * * Functions
		 * * Globals
		 */
		enum IdentifierKind get_identifier_kind(std::string id_name)
		{
			if (locals.count(id_name)) return IdentifierKind::LOCAL;
			if (parameters.count(id_name)) return IdentifierKind::PARAMETER;
			if (functions.count(id_name)) return IdentifierKind::FUNCTION;
			if (globals.count(id_name)) return IdentifierKind::GLOBAL;
			return IdentifierKind::UNDEFINED;
		}

		/**
		 * @brief Adds a class to the current compilation context.
		 * @param class_name The name of the class.
		 * @param class_type The type of the class.
		 * @returns A boolean indicating whether the class was added.
		 * A class is only added if it does not already exist.
		 */
		bool add_class(std::string class_name, Class class_type)
		{
			if (classes.count(class_name)) return false;

			classes[class_name] = class_type;

			// Add the class to the debugger symbols

			if (debug) {
				// TODO: create
			}

			return true;
		}

		/**
		 * @brief Adds a global variable to the current compilation context.
		 * @param global_name The name of the global variable.
		 * @param global_type The type of the global variable.
		 * @returns A boolean indicating whether the global variable was added.
		 * A global variable is only added if it does not already exist.
		 */
		bool add_global(std::string global_name, Type global_type)
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

		/**
		 * @brief Adds a function to the current compilation context.
		 * @param function_name The name of the function.
		 * @param function_type The type of the function.
		 * @returns A boolean indicating whether the function was added.
		 * A function is only added if it does not already exist.
		 */
		bool add_function(std::string function_name, Function function_type)
		{
			if (functions.count(function_name) || globals.count(function_name))
				return false;

			functions[function_name] = function_type;
			return true;
		}

		/**
		 * @brief Adds a parameter to the current function being compiled.
		 * @param param_name The name of the parameter.
		 * @param param_type The type of the parameter.
		 * @returns A boolean indicating whether the parameter was added.
		 * A parameter is only added if it does not already exist.
		 */
		bool add_parameter(std::string param_name, Type param_type)
		{
			if (parameters.count(param_name)) return false;

			parameters[param_name] = Variable(param_name, param_type, parameters_size);
			parameter_names_in_order.push_back(param_name);
			parameters_size += param_type.byte_size();
			return true;
		}

		/**
		 * @brief Adds a local variable to the current function being compiled.
		 * @param local_name The name of the local variable.
		 * @param local_type The type of the local variable.
		 * @returns A boolean indicating whether the local variable was added.
		 * A local variable is only added if it does not already exist.
		 */
		bool add_local(std::string local_name, Type local_type)
		{
			if (locals.count(local_name)) return false;

			locals[local_name] = Variable(local_name, local_type, locals_size);
			local_names_in_order.push_back(local_name);
			locals_size += local_type.storage_size();
			return true;
		}

		/**
		 * @brief Ends the scope of the current function being compiled.
		 * Clears all locals and parameters.
		 * In debug mode, the function, as well as its locals and
		 * parameters, arr also added to the debugger symbols.
		 */
		void end_function_scope()
		{
			// Add the function to the debugger symbols

			if (debug) {
				DebuggerFunction fn_symbols;

				// Add params

				for (const std::string& param_name : parameter_names_in_order) {
					DebuggerSymbolTypes fn_param_type = parameters[param_name].id.type.to_debug_type();
					fn_symbols.params.push_back(DebuggerSymbol(param_name, fn_param_type));
				}

				// Add locals

				for (const std::string& local_name : local_names_in_order) {
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

		/**
		 * @brief Gets the type of an identifier.
		 * The identifier is looked up in the current
		 * compilation context in the following order:
		 *
		 * * Local variables
		 * * Parameters
		 * * Global variables
		 *
		 * If the identifier is not found in any of these locations,
		 * an empty type is returned.
		 *
		 * @param id_name The name of the identifier to get.
		 * @returns The type of the identifier.
		 */
		Type get_type_of_identifier(std::string id_name)
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

		/**
		 * @brief Generates a label name that can be used to jump to.
		 * @param type The type of label to generate.
		 * @returns The generated label name.
		 */
		std::string generate_label(std::string type)
		{
			std::string label = "compiler-generated-label-";
			label += std::to_string(label_id++);
			label += "-for-";
			label += type;
			return label;
		}
};

#endif