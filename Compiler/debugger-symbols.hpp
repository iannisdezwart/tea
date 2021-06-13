#ifndef TEA_COMPILER_DEBUGGER_SYMBOLS_HEADER
#define TEA_COMPILER_DEBUGGER_SYMBOLS_HEADER

#include <bits/stdc++.h>
#include "util.hpp"

using namespace std;

string peek_line(ifstream& stream)
{
	string line;
	size_t pos = stream.tellg();
	getline(stream, line);
	stream.seekg(pos);
	return line;
}

struct ParsedLine {
	size_t indent;
	string line;
};

ParsedLine parse_line(const string& str)
{
	size_t i = 0;

	while (i < str.size()) {
		if (str[i] != '\t') break;
		i++;
	}

	return { i, str.substr(i) };
}

struct IndentFileNode {
	string line;
	vector<IndentFileNode *> children;

	IndentFileNode(const string& line) : line(line) {}
	void add_child(IndentFileNode *child) { children.push_back(child); }

	void dfs(function<void (IndentFileNode *, size_t)> callback, size_t depth = 0)
	{
		callback(this, depth);

		for (IndentFileNode *child : children) {
			child->dfs(callback, depth + 1);
		}
	}

	void dfs_after(function<void (IndentFileNode *, size_t)> callback, size_t depth = 0)
	{
		for (IndentFileNode *child : children) {
			child->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}
};

struct IndentFileRootNode : public IndentFileNode {
	IndentFileRootNode(const string& line) : IndentFileNode(line) {}

	~IndentFileRootNode()
	{
		dfs_after([this](IndentFileNode *node, size_t) {
			if (node != this) delete node;
		});
	}
};

class IndentFileParser {
	private:
		ifstream stream;

	public:
		ssize_t depth = -1;

		IndentFileParser(const string& file_name) : stream(file_name) {}

		IndentFileRootNode parse()
		{
			IndentFileRootNode root("root");
			vector<IndentFileNode *> node_stack = { &root };
			string line;

			while (getline(stream, line)) {
				ParsedLine next = parse_line(line);

				if (next.line == "") continue;
				if (next.indent > depth + 1) err("Illegally indented IndentFile");

				// Pop the node stack according to the new indentation

				size_t pop_count = depth - next.indent + 1;

				for (size_t i = 0; i < pop_count; i++) {
					node_stack.pop_back();
				}

				// Add the new node

				IndentFileNode *new_node = new IndentFileNode(next.line);
				node_stack.back()->add_child(new_node);
				node_stack.push_back(new_node);
				depth = next.indent;
			}

			return root;
		}
};

enum class DebuggerSymbolTypes : uint8_t {
	POINTER, U8, I8, U16, I16, U32, I32, U64, I64, USER_DEFINED_CLASS, UNDEFINED
};

const char *debugger_symbol_type_to_str(enum DebuggerSymbolTypes type)
{
	switch (type) {
		case DebuggerSymbolTypes::POINTER: return "POINTER";
		case DebuggerSymbolTypes::U8: return "U8";
		case DebuggerSymbolTypes::I8: return "I8";
		case DebuggerSymbolTypes::U16: return "U16";
		case DebuggerSymbolTypes::I16: return "I16";
		case DebuggerSymbolTypes::U32: return "U32";
		case DebuggerSymbolTypes::I32: return "I32";
		case DebuggerSymbolTypes::U64: return "U64";
		case DebuggerSymbolTypes::I64: return "I64";
		case DebuggerSymbolTypes::USER_DEFINED_CLASS: return "class";
		default: return "UNDEFINED";
	}
}

enum DebuggerSymbolTypes str_to_debugger_symbol_type(const string& str)
{
	if (str == "POINTER") return DebuggerSymbolTypes::POINTER;
	if (str == "U8") return DebuggerSymbolTypes::U8;
	if (str == "I8") return DebuggerSymbolTypes::I8;
	if (str == "U16") return DebuggerSymbolTypes::U16;
	if (str == "I16") return DebuggerSymbolTypes::I16;
	if (str == "U32") return DebuggerSymbolTypes::U32;
	if (str == "I32") return DebuggerSymbolTypes::I32;
	if (str == "U64") return DebuggerSymbolTypes::U64;
	if (str == "I64") return DebuggerSymbolTypes::I64;
	if (str == "class") return DebuggerSymbolTypes::USER_DEFINED_CLASS;
	return DebuggerSymbolTypes::UNDEFINED;
}

struct DebuggerSymbol {
	string name;
	enum DebuggerSymbolTypes type;

	DebuggerSymbol(const string& name, enum DebuggerSymbolTypes type)
		: name(name), type(type) {}

	size_t byte_size() const
	{
		switch (type) {
			case DebuggerSymbolTypes::POINTER: return 8;
			case DebuggerSymbolTypes::U8: return 1;
			case DebuggerSymbolTypes::I8: return 1;
			case DebuggerSymbolTypes::U16: return 2;
			case DebuggerSymbolTypes::I16: return 2;
			case DebuggerSymbolTypes::U32: return 4;
			case DebuggerSymbolTypes::I32: return 4;
			case DebuggerSymbolTypes::U64: return 8;
			case DebuggerSymbolTypes::I64: return 8;
			default: return 0;
		}
	}

	string to_str() const
	{
		string str;

		str += debugger_symbol_type_to_str(type);
		str += ' ';
		str += name;

		return str;
	}

	static DebuggerSymbol from_str(const string& str)
	{
		size_t space_index = str.find_first_of(' ');

		return DebuggerSymbol(str.substr(space_index + 1),
			str_to_debugger_symbol_type(str.substr(0, space_index)));
	}
};

struct DebuggerFunction {
	vector<DebuggerSymbol> params;
	vector<DebuggerSymbol> locals;
	vector<DebuggerSymbol> globals;
};

class DebuggerSymbols {
	public:
		map<string, DebuggerFunction> functions;
		vector<DebuggerSymbol> globals;

		void add_function(const string& name, const DebuggerFunction& symbols)
		{
			functions[name] = symbols;
		}

		void add_global(const DebuggerSymbol& symbol)
		{
			globals.push_back(symbol);
		}

		void build(const string& exec_file_name)
		{
			ofstream stream(exec_file_name + ".debug");

			// Functions

			if (functions.size()) {
				stream << "functions\n";
			}

			for (const pair<string, DebuggerFunction>& function : functions) {
				const string& fn_name = function.first;
				const DebuggerFunction& fn_symbols = function.second;
				const vector<DebuggerSymbol>& params = fn_symbols.params;
				const vector<DebuggerSymbol>& locals = fn_symbols.locals;

				// Function

				stream << '\t' << fn_name << '\n';

				// Parameters

				if (params.size()) {
					stream << "\t\tparameters\n";
				}

				for (const DebuggerSymbol& param : params) {
					stream << "\t\t\t" << param.to_str() << '\n';
				}

				// Locals

				if (locals.size()) {
					stream << "\t\tlocals\n";
				}

				for (const DebuggerSymbol& local : locals) {
					stream << "\t\t\t" << local.to_str() << '\n';
				}

			}

			// Globals

			if (globals.size()) {
				stream << "globals\n";
			}

			for (const DebuggerSymbol& global : globals) {
				stream << '\t' << global.to_str() << '\n';
			}

			stream.close();
		}

		static DebuggerSymbols parse(const string& file_name)
		{
			IndentFileParser parser(file_name);
			IndentFileNode file = parser.parse();
			DebuggerSymbols debugger_symbols;

			for (IndentFileNode *section : file.children) {
				// Scan function symbols

				if (section->line == "functions") {
					for (IndentFileNode *fn_node : section->children) {
						const string& fn_name = fn_node->line;
						DebuggerFunction fn_symbols;
						vector<DebuggerSymbol>& params = fn_symbols.params;
						vector<DebuggerSymbol>& locals = fn_symbols.locals;

						for (IndentFileNode *fn_section: fn_node->children) {
							// Scan function params

							if (fn_section->line == "parameters") {
								for (IndentFileNode *fn_param_node : fn_section->children) {
									const string& param_str = fn_param_node->line;
									params.push_back(DebuggerSymbol::from_str(param_str));
								}
							}

							// Scan function locals

							else if (fn_section->line == "locals") {

								for (IndentFileNode *fn_local_node : fn_section->children) {
									const string& local_str = fn_local_node->line;
									locals.push_back(DebuggerSymbol::from_str(local_str));
								}
							}
						}

						// Add the function to the debugger symbols

						debugger_symbols.add_function(fn_name, fn_symbols);
					}
				}

				// Scan global variables

				else if (section->line == "globals") {
					for (IndentFileNode *global : section->children) {
						const string& global_str = global->line;
						DebuggerSymbol global_sym = DebuggerSymbol::from_str(global_str);
						debugger_symbols.add_global(global_sym);
					}
				}
			}

			return debugger_symbols;
		}
};

#endif