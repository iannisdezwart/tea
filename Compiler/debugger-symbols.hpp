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

enum class FunctionArgTypes : uint8_t {
	POINTER, U8, I8, U16, I16, U32, I32, U64, I64, UNDEFINED
};

const char *function_arg_type_to_str(enum FunctionArgTypes type)
{
	switch (type) {
		case FunctionArgTypes::POINTER: return "POINTER";
		case FunctionArgTypes::U8: return "U8";
		case FunctionArgTypes::I8: return "I8";
		case FunctionArgTypes::U16: return "U16";
		case FunctionArgTypes::I16: return "I16";
		case FunctionArgTypes::U32: return "U32";
		case FunctionArgTypes::I32: return "I32";
		case FunctionArgTypes::U64: return "U64";
		case FunctionArgTypes::I64: return "I64";
		default: return "UNDEFINED";
	}
}

enum FunctionArgTypes str_to_function_arg_type(const string& str)
{
	if (str == "POINTER") return FunctionArgTypes::POINTER;
	if (str == "U8") return FunctionArgTypes::U8;
	if (str == "I8") return FunctionArgTypes::I8;
	if (str == "U16") return FunctionArgTypes::U16;
	if (str == "I16") return FunctionArgTypes::I16;
	if (str == "U32") return FunctionArgTypes::U32;
	if (str == "I32") return FunctionArgTypes::I32;
	if (str == "U64") return FunctionArgTypes::U64;
	if (str == "I64") return FunctionArgTypes::I64;
	return FunctionArgTypes::UNDEFINED;
}

struct FunctionArg {
	string name;
	enum FunctionArgTypes type;

	FunctionArg(const string& name, enum FunctionArgTypes type)
		: name(name), type(type) {}

	size_t byte_size() const
	{
		switch (type) {
			case FunctionArgTypes::POINTER: return 8;
			case FunctionArgTypes::U8: return 1;
			case FunctionArgTypes::I8: return 1;
			case FunctionArgTypes::U16: return 2;
			case FunctionArgTypes::I16: return 2;
			case FunctionArgTypes::U32: return 4;
			case FunctionArgTypes::I32: return 4;
			case FunctionArgTypes::U64: return 8;
			case FunctionArgTypes::I64: return 8;
			default: return 0;
		}
	}

	static FunctionArg from_str(const string& str)
	{
		size_t space_index = str.find_first_of(' ');

		return FunctionArg(str.substr(space_index + 1),
			str_to_function_arg_type(str.substr(0, space_index)));
	}
};

class DebuggerSymbols {
	public:
		map<string, vector<FunctionArg>> functions;

		void add_function(const string& fn_name, const vector<FunctionArg>& args)
		{
			functions[fn_name] = args;
		}

		void build(const string& exec_file_name)
		{
			ofstream stream(exec_file_name + ".debug");

			// Functions

			stream << "functions\n";

			for (const pair<string, vector<FunctionArg>>& function : functions) {
				const string& fn_name = function.first;
				const vector<FunctionArg>& args = function.second;

				stream << "\t" << fn_name << "\n";

				for (const FunctionArg& arg : args) {
					const char *arg_type = function_arg_type_to_str(arg.type);
					stream << "\t\t" << arg_type << " " << arg.name << "\n";
				}
			}

			stream.close();
		}

		static DebuggerSymbols parse(const string& file_name)
		{
			IndentFileParser parser(file_name);
			IndentFileNode file = parser.parse();
			DebuggerSymbols debugger_symbols;

			for (IndentFileNode *child : file.children) {
				if (child->line == "functions") {
					// Scan function symbols

					for (IndentFileNode *function_child : child->children) {
						const string& fn_name = function_child->line;
						vector<FunctionArg> args;

						// Scan function arguments

						for (IndentFileNode *function_arg : function_child->children) {
							const string& arg_str = function_arg->line;
							args.push_back(FunctionArg::from_str(arg_str));
						}

						// Add the function to the debugger symbols

						debugger_symbols.add_function(fn_name, args);
					}
				}
			}

			return debugger_symbols;
		}
};

#endif