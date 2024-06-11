#ifndef TEA_COMPILER_STATE_HEADER
#define TEA_COMPILER_STATE_HEADER

#include <deque>

#include "Compiler/ASTNodes/AST.hpp"
#include "Compiler/util.hpp"
#include "VM/cpu.hpp"
#include "Compiler/debugger-symbols.hpp"
#include "Compiler/type.hpp"
#include "util.hpp"

/**
 * @brief Structure for the compiler state.
 * Holds all the information about the current compilation process.
 * This includes all the identifiers, types, and functions in the current
 * compilation context, as well as the current stack frame.
 */
struct TypeCheckState
{
	// The map of names by ID, from the parsing phase.
	const std::unordered_map<uint, std::string> &names_by_id;

	// AST reference.
	const AST &ast;

	// A map of all functions in the current compilation context.
	std::unordered_map<uint, FunctionSignature> functions;

	// A map of all classes in the current compilation context.
	std::unordered_map<uint, ClassDefinition> classes;

	// A map of all global variables in the current compilation context.
	std::unordered_map<uint, VariableDefinition> globals;

	// The size of all global variables combined
	// in the current compilation context.
	uint globals_size = 0;

	// The current function being compiled.
	std::optional<uint> current_function_id;

	// A map of all parameters in the current function being compiled.
	std::unordered_map<uint, VariableDefinition> parameters;

	// The parameters in the current function being compiled, in order.
	std::vector<uint> parameter_ids_in_order;

	// The size of all parameters combined in the
	// current function being compiled.
	uint parameters_size = 0;

	// A map of all local variables in the current function being compiled.
	std::deque<std::unordered_map<uint, VariableDefinition>> locals;

	// Debugger symbols for the local variables in the current function being
	// compiled, in order.
	std::vector<DebuggerSymbol> local_symbols;

	// The size of all local variables combined in the
	// current function being compiled.
	uint locals_size = 0;

	// Whether debug symbols should be generated.
	const bool debug;

	// The debugger symbols.
	DebuggerSymbols debugger_symbols;

	/**
	 * @brief Constructs a new Compiler State object.
	 * @param debug Whether debug symbols should be generated.
	 */
	TypeCheckState(bool debug, const AST &ast, const std::unordered_map<uint, std::string> &names_by_id)
		: names_by_id(names_by_id), ast(ast), debug(debug) {}

	IdentifierKind
	get_identifier_kind(uint id)
	{
		for (const std::unordered_map<uint, VariableDefinition> &frame : locals)
		{
			if (frame.find(id) != frame.end())
				return IdentifierKind::LOCAL;
		}
		if (parameters.find(id) != parameters.end())
			return IdentifierKind::PARAMETER;
		if (functions.find(id) != functions.end())
			return IdentifierKind::FUNCTION;
		if (globals.find(id) != globals.end())
			return IdentifierKind::GLOBAL;
		return IdentifierKind::UNDEFINED;
	}

	/**
	 * @brief Defines a class to the current compilation context.
	 * @param class_name The name of the class.
	 * @returns A boolean indicating whether the class was added.
	 * A class is only added if it does not already exist.
	 */
	bool
	def_class(uint class_id)
	{
		if (classes.count(class_id))
			return false;

		classes[class_id] = ClassDefinition();
		return true;
	}

	/**
	 * @brief Adds a defined class to the current compilation context.
	 * @param class_name The name of the class.
	 * @param class_definition The type of the class.
	 */
	void
	add_class(uint class_id, ClassDefinition class_def, const std::vector<uint> &extra_data)
	{
		classes[class_id] = class_def;

		// Add the class to the debugger symbols

		if (debug)
		{
			DebuggerClass debugger_class;

			debugger_class.fields.reserve(class_def.fields.size());
			for (const IdentifierDefinition &field : class_def.fields)
				debugger_class.fields.push_back(
					DebuggerSymbol(names_by_id.at(field.id), field.type.to_debug_type(extra_data),
						names_by_id.at(field.type.value)));

			debugger_symbols.add_class(names_by_id.at(class_id), debugger_class);
		}
	}

	/**
	 * @brief Introduces a new local scope to the current compilation context.
	 */
	void
	begin_local_scope()
	{
		locals.push_back(std::unordered_map<uint, VariableDefinition>());
	}

	/**
	 * @brief Ends the current local scope in the compilation context.
	 */
	void
	end_local_scope()
	{
		locals.pop_back();
	}

	bool
	add_local(uint local_id, Type local_type, const std::vector<uint> &extra_data)
	{
		if (locals.back().find(local_id) != locals.back().end())
			return false;

		locals.back()[local_id] = VariableDefinition(local_id, local_type, locals_size);

		if (debug)
		{
			local_symbols.push_back(DebuggerSymbol(
				names_by_id.at(local_id),
				local_type.to_debug_type(extra_data),
				names_by_id.at(local_type.value)));
		}
		locals_size += local_type.storage_size(extra_data);
		return true;
	}

	VariableDefinition
	get_local(uint local_id)
	{
		for (const std::unordered_map<uint, VariableDefinition> &frame : locals)
		{
			if (frame.count(local_id))
				return frame.at(local_id);
		}

		err("Local variable \"%s\" not found", names_by_id.at(local_id).c_str());
	}

	bool
	add_global(uint global_id, Type global_type, const std::vector<uint> &extra_data)
	{
		if (globals.find(global_id) != globals.end() || functions.find(global_id) != functions.end())
			return false;

		globals[global_id] = VariableDefinition(global_id, global_type, globals_size);
		globals_size += global_type.storage_size(extra_data);

		// Add the global to the debugger symbols

		if (debug)
		{
			debugger_symbols.add_global(DebuggerSymbol(
				names_by_id.at(global_id),
				global_type.to_debug_type(extra_data),
				names_by_id.at(global_type.value)));
		}

		return true;
	}

	bool
	add_var(uint decl_id, Type type, const std::vector<uint> &extra_data)
	{
		if (current_function_id.has_value())
			return add_local(decl_id, type, extra_data);
		return add_global(decl_id, type, ast.extra_data);
	}

	bool
	add_function(uint function_id, FunctionSignature function_type)
	{
		if (functions.find(function_id) != functions.end() || globals.find(function_id) != globals.end())
			return false;

		functions[function_id] = function_type;
		return true;
	}

	bool
	add_parameter(uint param_id, Type param_type, const std::vector<uint> &extra_data)
	{
		if (parameters.find(param_id) != parameters.end())
			return false;

		parameters[param_id] = VariableDefinition(param_id, param_type, parameters_size);
		parameter_ids_in_order.push_back(param_id);
		parameters_size += param_type.byte_size(extra_data);
		return true;
	}

	void
	begin_function_scope(uint function_id)
	{
		if (current_function_id.has_value())
		{
			err("Cannot begin function scope while another function is being compiled");
		}

		current_function_id.emplace(function_id);
	}

	/**
	 * @brief Ends the scope of the current function being compiled.
	 * Clears all locals and parameters.
	 * In debug mode, the function, as well as its locals and
	 * parameters, are also added to the debugger symbols.
	 */
	void
	end_function_scope(const std::vector<uint> &extra_data)
	{
		if (!current_function_id.has_value())
		{
			err("Cannot end function scope while no function is being compiled");
		}

		// Add the function to the debugger symbols

		if (debug)
		{
			DebuggerFunction fn_symbols;

			// Add params

			for (const uint &param_id : parameter_ids_in_order)
			{
				DebuggerSymbolType fn_param_type = parameters[param_id].id.type.to_debug_type(extra_data);
				fn_symbols.params.push_back(DebuggerSymbol(
					names_by_id.at(param_id),
					fn_param_type,
					names_by_id.at(parameters[param_id].id.type.value)));
			}

			// Add locals

			fn_symbols.locals = local_symbols;

			debugger_symbols.add_function(names_by_id.at(current_function_id.value()), fn_symbols);
		}

		current_function_id.reset();

		locals.clear();
		local_symbols.clear();
		locals_size = 0;

		parameters.clear();
		parameter_ids_in_order.clear();
		parameters_size = 0;
	}

	Type
	get_type_of_identifier(uint id)
	{
		IdentifierKind id_kind = get_identifier_kind(id);

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
			for (const std::unordered_map<uint, VariableDefinition> &frame : locals)
			{
				if (frame.find(id) != frame.end())
					return frame.at(id).id.type;
			}

		case IdentifierKind::PARAMETER:
			return parameters[id].id.type;

		case IdentifierKind::GLOBAL:
			return globals[id].id.type;

		default:
			return Type();
		}
	}
};

#endif