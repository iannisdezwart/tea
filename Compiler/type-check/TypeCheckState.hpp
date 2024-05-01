#ifndef TEA_COMPILER_STATE_HEADER
#define TEA_COMPILER_STATE_HEADER

#include <stack>

#include "Compiler/util.hpp"
#include "VM/cpu.hpp"
#include "Compiler/debugger-symbols.hpp"
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
	// The name of this identifier.
	std::string name;

	// The data type of this identifier.
	Type type;

	IdentifierDefinition() {}
	IdentifierDefinition(std::string name, Type type)
		: name(std::move(name)), type(std::move(type)) {}
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
	size_t offset;

	// Holds the name and type of this variable.
	IdentifierDefinition id;

	VariableDefinition() {}
	VariableDefinition(const std::string &name, Type &type, size_t offset)
		: offset(offset),
		  id(name, type) {}
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
	FunctionSignature(const std::string &fn_name, Type &return_type)
		: id(fn_name, return_type) {}

	/**
	 * @brief Adds a parameter to this function.
	 * @param param_name The name of the parameter.
	 * @param param_type The type of the parameter.
	 */
	void
	add_parameter(const std::string &param_name, const Type &param_type)
	{
		parameters.push_back(IdentifierDefinition(param_name, param_type));
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
	size_t byte_size;

	// A list of all fields in the class.
	std::vector<IdentifierDefinition> fields;

	ClassDefinition() {}
	ClassDefinition(size_t byte_size)
		: byte_size(byte_size) {}

	/**
	 * @brief Adds a field to this class.
	 * @param field_name The name of the field.
	 * @param field_type The type of the field.
	 */
	void
	add_field(const std::string &field_name, const Type &field_type)
	{
		fields.push_back(IdentifierDefinition(field_name, field_type));
	}

	/**
	 * @param field_name The field name to look for.
	 * @returns The type of a field.
	 */
	const Type &
	get_field_type(const std::string &field_name) const
	{
		for (const IdentifierDefinition &field : fields)
		{
			if (field.name == field_name)
				return field.type;
		}

		err("Class field \"%s\" not found", field_name.c_str());
	}

	/**
	 * @param field_name The field name to look for.
	 * @returns The offset of a field.
	 */
	size_t
	get_field_offset(const std::string &field_name) const
	{
		size_t offset = 0;

		for (const IdentifierDefinition &field : fields)
		{
			if (field.name == field_name)
				return offset;

			offset += field.type.storage_size();
		}

		err("Class field \"%s\" not found", field_name.c_str());
	}

	/**
	 * @param field_name The field name to look for.
	 * @returns Whether a field exists in this class.
	 */
	bool
	has_field(const std::string &field_name) const
	{
		for (const IdentifierDefinition &field : fields)
		{
			if (field.name == field_name)
				return true;
		}

		return false;
	}
};

/**
 * @brief Structure for location information.
 * Holds the kind of identifier (global, function, parameter or local),
 * as well as the offset and byte size of the identifier.
 */
struct LocationData
{
	// The kind of identifier (global, function, parameter or local).
	IdentifierKind id_kind;

	// The offset of the identifier.
	// Depending on whether the identifier is a global, local or parameter,
	// the offset is either relative to the top of the stack,
	// or the start of the current stack frame.
	int64_t offset;

	LocationData() {}

	LocationData(IdentifierKind id_kind, int64_t offset)
		: id_kind(id_kind), offset(offset) {}

	/**
	 * @returns A boolean indicating whether this location is a global.
	 */
	bool
	is_at_stack_top()
		const
	{
		return id_kind == IdentifierKind::GLOBAL;
	}

	/**
	 * @returns A boolean indicating whether this location is
	 * a local or a parameter.
	 */
	bool
	is_at_frame_top()
		const
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
struct TypeCheckState
{
	// A map of all functions in the current compilation context.
	std::unordered_map<std::string, FunctionSignature> functions;

	// A map of all classes in the current compilation context.
	std::unordered_map<std::string, ClassDefinition> classes;

	// A map of all global variables in the current compilation context.
	std::unordered_map<std::string, VariableDefinition> globals;

	// The size of all global variables combined
	// in the current compilation context.
	uint64_t globals_size = 0;

	// The current function name being compiled.
	std::optional<std::string> current_function_name;

	// A map of all parameters in the current function being compiled.
	std::unordered_map<std::string, VariableDefinition> parameters;

	// The names of the parameters in the current
	// function being compiled, in order.
	std::vector<std::string> parameter_names_in_order;

	// The size of all parameters combined in the
	// current function being compiled.
	uint64_t parameters_size = 0;

	// A map of all local variables in the current function being compiled.
	std::unordered_map<std::string, VariableDefinition> locals;

	// The names of the local variables in the current
	// function being compiled, in order.
	std::vector<std::string> local_names_in_order;

	// The size of all local variables combined in the
	// current function being compiled.
	uint64_t locals_size = 0;

	// Whether debug symbols should be generated.
	const bool debug;

	// The debugger symbols.
	DebuggerSymbols debugger_symbols;

	/**
	 * @brief Constructs a new Compiler State object.
	 * @param debug Whether debug symbols should be generated.
	 */
	TypeCheckState(bool debug)
		: debug(debug) {}

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
	IdentifierKind
	get_identifier_kind(std::string id_name)
	{
		if (locals.count(id_name))
			return IdentifierKind::LOCAL;
		if (parameters.count(id_name))
			return IdentifierKind::PARAMETER;
		if (functions.count(id_name))
			return IdentifierKind::FUNCTION;
		if (globals.count(id_name))
			return IdentifierKind::GLOBAL;
		return IdentifierKind::UNDEFINED;
	}

	/**
	 * @brief Adds a class to the current compilation context.
	 * @param class_name The name of the class.
	 * @param class_type The type of the class.
	 * @returns A boolean indicating whether the class was added.
	 * A class is only added if it does not already exist.
	 */
	bool
	add_class(std::string class_name, ClassDefinition class_type)
	{
		if (classes.count(class_name))
			return false;

		classes[class_name] = class_type;

		// Add the class to the debugger symbols

		if (debug)
		{
			DebuggerClass debugger_class;

			debugger_class.fields.reserve(class_type.fields.size());
			for (const IdentifierDefinition &field : class_type.fields)
				debugger_class.fields.push_back(
					DebuggerSymbol(field.name, field.type.to_debug_type(),
						field.type.class_name));

			debugger_symbols.add_class(class_name, debugger_class);
		}

		return true;
	}

	/**
	 * @brief Adds a local variable to the current compilation context.
	 * @param local_name The name of the local variable.
	 * @param local_type The type of the local variable.
	 * @returns A boolean indicating whether the local variable was added.
	 * A local variable is only added if it does not already exist.
	 */
	bool
	add_local(std::string local_name, Type local_type)
	{
		if (locals.count(local_name))
			return false;

		locals[local_name] = VariableDefinition(local_name, local_type, locals_size);
		local_names_in_order.push_back(local_name);
		locals_size += local_type.storage_size();
		return true;
	}

	/**
	 * @brief Adds a global variable to the current compilation context.
	 * @param global_name The name of the global variable.
	 * @param global_type The type of the global variable.
	 * @returns A boolean indicating whether the global variable was added.
	 * A global variable is only added if it does not already exist.
	 */
	bool
	add_global(std::string global_name, Type global_type)
	{
		if (globals.count(global_name) || functions.count(global_name))
			return false;

		globals[global_name] = VariableDefinition(global_name, global_type, globals_size);
		globals_size += global_type.storage_size();

		// Add the global to the debugger symbols

		if (debug)
		{
			debugger_symbols.add_global(DebuggerSymbol(global_name,
				global_type.to_debug_type(), global_type.class_name));
		}

		return true;
	}

	/**
	 * @brief Adds a variable to the current context being compiled.
	 * @param decl_name The declaration name of the variable.
	 * @param type The type of the variable.
	 * @returns A boolean indicating whether the variable was succesfully added.
	 * A variable is only added if it does not already exist.
	 */
	bool
	add_var(std::string decl_name, Type type)
	{
		if (current_function_name.has_value())
			return add_local(decl_name, type);
		return add_global(decl_name, type);
	}

	/**
	 * @brief Adds a function to the current compilation context.
	 * @param function_name The name of the function.
	 * @param function_type The type of the function.
	 * @returns A boolean indicating whether the function was added.
	 * A function is only added if it does not already exist.
	 */
	bool
	add_function(std::string function_name, FunctionSignature function_type)
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
	bool
	add_parameter(std::string param_name, Type param_type)
	{
		if (parameters.count(param_name))
			return false;

		parameters[param_name] = VariableDefinition(param_name, param_type, parameters_size);
		parameter_names_in_order.push_back(param_name);
		parameters_size += param_type.byte_size();
		return true;
	}

	/**
	 * @brief Begins the scope of a new function being compiled.
	 * @param function_name The name of the function.
	 */
	void
	begin_function_scope(std::string function_name)
	{
		if (current_function_name.has_value())
		{
			err("Cannot begin function scope while another function is being compiled");
		}

		current_function_name.emplace(std::move(function_name));
	}

	/**
	 * @brief Ends the scope of the current function being compiled.
	 * Clears all locals and parameters.
	 * In debug mode, the function, as well as its locals and
	 * parameters, are also added to the debugger symbols.
	 */
	void
	end_function_scope()
	{
		if (!current_function_name.has_value())
		{
			err("Cannot end function scope while no function is being compiled");
		}

		// Add the function to the debugger symbols

		if (debug)
		{
			DebuggerFunction fn_symbols;

			// Add params

			for (const std::string &param_name : parameter_names_in_order)
			{
				DebuggerSymbolType fn_param_type = parameters[param_name].id.type.to_debug_type();
				std::string class_name           = parameters[param_name].id.type.class_name;
				fn_symbols.params.push_back(DebuggerSymbol(param_name, fn_param_type, class_name));
			}

			// Add locals

			for (const std::string &local_name : local_names_in_order)
			{
				DebuggerSymbolType local_type = locals[local_name].id.type.to_debug_type();
				std::string class_name        = locals[local_name].id.type.class_name;
				fn_symbols.locals.push_back(DebuggerSymbol(local_name, local_type, class_name));
			}

			debugger_symbols.add_function(current_function_name.value(), fn_symbols);
		}

		current_function_name.reset();

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
	Type
	get_type_of_identifier(std::string id_name)
	{
		IdentifierKind id_kind = get_identifier_kind(id_name);

		switch (id_kind)
		{
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
};

#endif