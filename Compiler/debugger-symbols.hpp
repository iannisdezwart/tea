#ifndef TEA_COMPILER_DEBUGGER_SYMBOLS_HEADER
#define TEA_COMPILER_DEBUGGER_SYMBOLS_HEADER

#include <map>
#include <fstream>

#include "Compiler/util.hpp"

/**
 * @brief Structure that holds information about a line
 * of the debugger symbols file.
 * Contains the indent size and the rest of the line excluding indent.
 */
struct ParsedLine
{
	// The indent size of the line.
	size_t indent;

	// The rest of the line excluding indent.
	std::string line;
};

/**
 * @brief Parses a line of the debugger symbols file.
 * Calculates the indentation and the rest of the line.
 * @param str The line to parse
 * @returns The parsed line.
 */
ParsedLine
parse_line(const std::string &str)
{
	size_t i = 0;

	while (i < str.size())
	{
		if (str[i] != '\t')
		{
			break;
		}

		i++;
	}

	return { i, str.substr(i) };
}

/**
 * @brief Represents a node in the debugger symbols file.
 * A node can have multiple subnodes.
 */
struct IndentFileNode
{
	// The line of the node.
	std::string line;

	// The children of the node.
	std::vector<IndentFileNode *> children;

	/**
	 * @brief Constructs a new indent file node.
	 * @param line The line of the node.
	 */
	IndentFileNode(const std::string &line)
		: line(line) {}

	/**
	 * @brief Adds a child to the node.
	 */
	void
	add_child(IndentFileNode *child)
	{
		children.push_back(child);
	}

	/**
	 * @brief Depth-first searches the node and calls the callback for
	 * each node. The callback is called before the children are searched.
	 */
	void
	dfs(std::function<void(IndentFileNode *, size_t)> callback, size_t depth = 0)
	{
		callback(this, depth);

		for (IndentFileNode *child : children)
		{
			child->dfs(callback, depth + 1);
		}
	}

	/**
	 * @brief Depth-first searches the node and calls the callback for
	 * each node. The callback is called after the children are searched.
	 */
	void
	dfs_after(std::function<void(IndentFileNode *, size_t)> callback, size_t depth = 0)
	{
		for (IndentFileNode *child : children)
		{
			child->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}
};

/**
 * @brief Represents the root of the debugger symbols file.
 */
struct IndentFileRootNode : public IndentFileNode
{
	/**
	 * @brief Constructs a new indent file root node.
	 * @param line The line of the node.
	 */
	IndentFileRootNode(const std::string &line)
		: IndentFileNode(line) {}

	/**
	 * @brief Deletes all children and the root node itself.
	 */
	~IndentFileRootNode()
	{
		auto cb = [this](IndentFileNode *node, size_t)
		{
			if (node != this)
			{
				delete node;
			}
		};

		dfs_after(cb);
	}
};

/**
 * @brief Class that parses the debugger symbols file.

 */
struct IndentFileParser
{
	// The file stream of the debugger symbols file.
	std::ifstream stream;

	// The current depth of the nodes.
	ssize_t depth = -1;

	/**
	 * @brief Constructs a new indent file parser.
	 * @param file The file to parse.
	 */
	IndentFileParser(const std::string &file_name)
		: stream(file_name) {}

	/**
	 * @brief Parses the file.
	 * @returns The root node of the file.
	 */
	IndentFileRootNode
	parse()
	{
		IndentFileRootNode root("root");
		std::vector<IndentFileNode *> node_stack = { &root };
		std::string line;

		// Read all lines of the file and adds them to the tree.

		while (getline(stream, line))
		{
			ParsedLine next = parse_line(line);

			if (next.line == "")
			{
				continue;
			}
			if (next.indent > depth + 1)
			{
				err("Illegally indented IndentFile");
			}

			// Pop the node stack according to the new indentation.

			size_t pop_count = depth - next.indent + 1;

			for (size_t i = 0; i < pop_count; i++)
			{
				node_stack.pop_back();
			}

			// Add the new node.

			IndentFileNode *new_node = new IndentFileNode(next.line);
			node_stack.back()->add_child(new_node);
			node_stack.push_back(new_node);
			depth = next.indent;
		}

		return root;
	}
};

/**
 * @brief Enum that represents the possible debugger symbol types.
 * TODO: See how to support user-defined classes in a nicer way.
 */
enum struct DebuggerSymbolType : uint8_t
{
	POINTER,
	U8,
	I8,
	U16,
	I16,
	U32,
	I32,
	U64,
	I64,
	USER_DEFINED_CLASS,
	UNDEFINED
};

/**
 * @brief Converts a debugger symbol type to a string.
 * @param type The debugger symbol type.
 * @returns A string representation of the debugger symbol type.
 */
const char *
debugger_symbol_type_to_str(DebuggerSymbolType type)
{
	switch (type)
	{
	case DebuggerSymbolType::POINTER:
		return "POINTER";
	case DebuggerSymbolType::U8:
		return "U8";
	case DebuggerSymbolType::I8:
		return "I8";
	case DebuggerSymbolType::U16:
		return "U16";
	case DebuggerSymbolType::I16:
		return "I16";
	case DebuggerSymbolType::U32:
		return "U32";
	case DebuggerSymbolType::I32:
		return "I32";
	case DebuggerSymbolType::U64:
		return "U64";
	case DebuggerSymbolType::I64:
		return "I64";
	case DebuggerSymbolType::USER_DEFINED_CLASS:
		return "CLASS";
	default:
		return "UNDEFINED";
	}
}

/**
 * @brief Converts a string to a debugger symbol type.
 * @param str The string to parse.
 * @returns The parsed debugger symbol type.
 */
DebuggerSymbolType
str_to_debugger_symbol_type(const std::string &str)
{
	if (str == "POINTER")
		return DebuggerSymbolType::POINTER;
	if (str == "U8")
		return DebuggerSymbolType::U8;
	if (str == "I8")
		return DebuggerSymbolType::I8;
	if (str == "U16")
		return DebuggerSymbolType::U16;
	if (str == "I16")
		return DebuggerSymbolType::I16;
	if (str == "U32")
		return DebuggerSymbolType::U32;
	if (str == "I32")
		return DebuggerSymbolType::I32;
	if (str == "U64")
		return DebuggerSymbolType::U64;
	if (str == "I64")
		return DebuggerSymbolType::I64;
	if (str == "CLASS")
		return DebuggerSymbolType::USER_DEFINED_CLASS;
	return DebuggerSymbolType::UNDEFINED;
}

/**
 * @brief A structure that represents a debugger symbol.
 */
struct DebuggerSymbol
{
	// The name of the symbol.
	std::string name;

	// The type of the symbol.
	DebuggerSymbolType type;

	// Extra data.
	std::string extra;

	/**
	 * @brief Constructs a new debugger symbol.
	 * @param name The name of the symbol.
	 * @param type The type of the symbol.
	 * @param extra Extra data.
	 */
	DebuggerSymbol(const std::string &name, DebuggerSymbolType type,
		const std::string &extra = "")
		: name(name), type(type), extra(extra) {}

	/**
	 * @brief Returns the byte size of the symbol.
	 * TODO: See how to support user-defined classes.
	 * @returns The byte size of the symbol.
	 */
	size_t
	byte_size() const
	{
		switch (type)
		{
		case DebuggerSymbolType::POINTER:
			return 8;
		case DebuggerSymbolType::U8:
		case DebuggerSymbolType::I8:
			return 1;
		case DebuggerSymbolType::U16:
		case DebuggerSymbolType::I16:
			return 2;
		case DebuggerSymbolType::U32:
		case DebuggerSymbolType::I32:
			return 4;
		case DebuggerSymbolType::U64:
		case DebuggerSymbolType::I64:
			return 8;
		default:
			return 0;
		}
	}

	/**
	 * @brief Returns the string representation of the symbol.
	 * @returns The string representation of the symbol.
	 */
	std::string
	to_str() const
	{
		std::string str;

		str += debugger_symbol_type_to_str(type);
		if (extra.size())
		{
			str += ':';
			str += extra;
		}
		str += ' ';
		str += name;

		return str;
	}

	/**
	 * @brief Converts a string to a debugger symbol.
	 * @param str The string to parse.
	 * @returns The parsed debugger symbol.
	 */
	static DebuggerSymbol
	from_str(const std::string &str)
	{
		size_t space_index         = str.find_first_of(' ');
		std::string name           = str.substr(space_index + 1);
		std::string type_and_extra = str.substr(0, space_index);
		size_t colon_index         = type_and_extra.find_first_of(':');
		std::string extra;
		DebuggerSymbolType type;

		if (colon_index != std::string::npos)
		{
			extra = type_and_extra.substr(colon_index + 1);
			type  = str_to_debugger_symbol_type(str.substr(0, colon_index));
		}
		else
		{
			type = str_to_debugger_symbol_type(type_and_extra);
		}

		return DebuggerSymbol(name, type, extra);
	}
};

/**
 * @brief A structure that represents a debugger function.
 * Holds the parameters and locals of the function.
 */
struct DebuggerFunction
{
	std::vector<DebuggerSymbol> params;
	std::vector<DebuggerSymbol> locals;
};

/**
 * @brief A structure that represents a debugger class.
 * Holds the fields of the class.
 */
struct DebuggerClass
{
	std::vector<DebuggerSymbol> fields;
};

/**
 * @brief A structure that reprsents the debugger symbols.
 */
struct DebuggerSymbols
{
	// A map containing the function debugger symbols.
	std::map<std::string, DebuggerFunction> functions;

	// A list containing the global debugger symbols.
	std::vector<DebuggerSymbol> globals;

	// A list containing the class debugger symbols.
	std::map<std::string, DebuggerClass> classes;

	/**
	 * @brief Adds a class to the debugger symbols.
	 */
	void
	add_class(const std::string &name, const DebuggerClass &cls)
	{
		classes[name] = cls;
	}

	/**
	 * @brief Adds a function to the debugger symbols.
	 * @param name The name of the function.
	 * @param symbols The function debugger type.
	 */
	void
	add_function(const std::string &name, const DebuggerFunction &symbols)
	{
		functions[name] = symbols;
	}

	/**
	 * @brief Adds a global to the debugger symbols.
	 * @param symbol The global debugger symbol.
	 */
	void
	add_global(const DebuggerSymbol &symbol)
	{
		globals.push_back(symbol);
	}

	/**
	 * @brief Builds the debugger symbols and writes them to
	 * a debugger symbols file.
	 * @param exec_file_name The name of the executable.
	 * The symbols file will be named after the executable with
	 * the extension `.debug`.
	 */
	void
	build(const std::string &exec_file_name)
	{
		std::ofstream stream(exec_file_name + ".debug");

		// Classes

		if (classes.size())
		{
			stream << "classes\n";
		}

		for (const auto &[cls_name, cls_symbols] : classes)
		{
			const std::vector<DebuggerSymbol> &fields = cls_symbols.fields;

			// Class

			stream << '\t' << cls_name << '\n';

			// Fields

			if (fields.size())
			{
				stream << "\t\tfields\n";
			}

			for (const DebuggerSymbol &field : fields)
			{
				stream << "\t\t\t" << field.to_str() << '\n';
			}
		}

		// Functions

		if (functions.size())
		{
			stream << "functions\n";
		}

		for (const auto &[fn_name, fn_symbols] : functions)
		{
			const std::vector<DebuggerSymbol> &params = fn_symbols.params;
			const std::vector<DebuggerSymbol> &locals = fn_symbols.locals;

			// Function

			stream << '\t' << fn_name << '\n';

			// Parameters

			if (params.size())
			{
				stream << "\t\tparameters\n";
			}

			for (const DebuggerSymbol &param : params)
			{
				stream << "\t\t\t" << param.to_str() << '\n';
			}

			// Locals

			if (locals.size())
			{
				stream << "\t\tlocals\n";
			}

			for (const DebuggerSymbol &local : locals)
			{
				stream << "\t\t\t" << local.to_str() << '\n';
			}
		}

		// Globals

		if (globals.size())
		{
			stream << "globals\n";
		}

		for (const DebuggerSymbol &global : globals)
		{
			stream << '\t' << global.to_str() << '\n';
		}

		stream.close();
	}

	static void
	scan_function_params(IndentFileNode *fn_section,
		std::vector<DebuggerSymbol> &params)
	{
		for (IndentFileNode *fn_param_node : fn_section->children)
		{
			const std::string &param_str = fn_param_node->line;
			params.push_back(DebuggerSymbol::from_str(param_str));
		}
	}

	static void
	scan_function_locals(IndentFileNode *fn_section,
		std::vector<DebuggerSymbol> &locals)
	{
		for (IndentFileNode *fn_local_node : fn_section->children)
		{
			const std::string &local_str = fn_local_node->line;
			locals.push_back(DebuggerSymbol::from_str(local_str));
		}
	}

	static void
	scan_classes(IndentFileNode *section, DebuggerSymbols &debugger_symbols)
	{
		for (IndentFileNode *cls_node : section->children)
		{
			const std::string &cls_name = cls_node->line;
			DebuggerClass cls_symbols;

			for (IndentFileNode *cls_section : cls_node->children)
			{
				if (cls_section->line == "fields")
				{
					for (IndentFileNode *field_node : cls_section->children)
					{
						const std::string &field_str = field_node->line;
						cls_symbols.fields.push_back(DebuggerSymbol::from_str(field_str));
					}
				}
			}

			// Add the class to the debugger symbols

			debugger_symbols.add_class(cls_name, cls_symbols);
		}
	}

	static void
	scan_functions(IndentFileNode *section, DebuggerSymbols &debugger_symbols)
	{
		for (IndentFileNode *fn_node : section->children)
		{
			const std::string &fn_name = fn_node->line;
			DebuggerFunction fn_symbols;
			std::vector<DebuggerSymbol> &params = fn_symbols.params;
			std::vector<DebuggerSymbol> &locals = fn_symbols.locals;

			for (IndentFileNode *fn_section : fn_node->children)
			{
				if (fn_section->line == "parameters")
				{
					scan_function_params(fn_section, params);
				}
				else if (fn_section->line == "locals")
				{
					scan_function_locals(fn_section, locals);
				}
			}

			// Add the function to the debugger symbols

			debugger_symbols.add_function(fn_name, fn_symbols);
		}
	}

	static void
	scan_globals(IndentFileNode *section, DebuggerSymbols &debugger_symbols)
	{
		for (IndentFileNode *global : section->children)
		{
			const std::string &global_str = global->line;
			DebuggerSymbol global_sym     = DebuggerSymbol::from_str(global_str);
			debugger_symbols.add_global(global_sym);
		}
	}

	/**
	 * @brief Parses a debugger symbols file.
	 * @param file_name The file name of the symbols file.
	 * @returns The parsed debugger symbols.
	 */
	static DebuggerSymbols
	parse(const std::string &file_name)
	{
		IndentFileParser parser(file_name);
		IndentFileNode file = parser.parse();
		DebuggerSymbols debugger_symbols;

		for (IndentFileNode *section : file.children)
		{
			// Scan classes

			if (section->line == "classes")
			{
				scan_classes(section, debugger_symbols);
			}

			// Scan function symbols

			if (section->line == "functions")
			{
				scan_functions(section, debugger_symbols);
			}

			// Scan global variables

			else if (section->line == "globals")
			{
				scan_globals(section, debugger_symbols);
			}
		}

		return debugger_symbols;
	}
};

#endif