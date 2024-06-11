#ifndef TEA_AST_NODE_SYS_CALL_HEADER_HEADER
#define TEA_AST_NODE_SYS_CALL_HEADER_HEADER

#include <set>
#include <string>

#include "Compiler/ASTNodes/ASTFunctions-fwd.hpp"
#include "Compiler/ASTNodes/AST.hpp"

SysCallId
str_to_sys_call_id(const std::string &s)
{
	if (s == "PRINT_CHAR")
	{
		return SysCallId::PRINT_CHAR;
	}

	if (s == "GET_CHAR")
	{
		return SysCallId::GET_CHAR;
	}

	return SysCallId::UNKNOWN;
}

std::string
sys_call_id_to_str(SysCallId sys_call_id)
{
	switch (sys_call_id)
	{
	case SysCallId::PRINT_CHAR:
		return "PRINT_CHAR";

	case SysCallId::GET_CHAR:
		return "GET_CHAR";

	default:
		return "UNKNOWN";
	}
}

void
sys_call_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth)
{
	uint arguments_ed_idx = ast.data[node].sys_call.arguments_ed_idx;
	uint len              = ast.extra_data[arguments_ed_idx];
	for (uint i = arguments_ed_idx + 1; i < arguments_ed_idx + 1 + len; i++)
	{
		uint argument_node = ast.extra_data[i];
		ast_dfs(ast, argument_node, callback, depth + 1);
	}
	callback(node, depth);
}

std::string
sys_call_to_str(const AST &ast, uint node)
{
	std::string s = "SysCall { name = ";
	s += sys_call_id_to_str(static_cast<SysCallId>(ast.data[node].sys_call.sys_call_id));
	s += " } @ ";
	s += std::to_string(node);
	return s;
}

void
sys_call_type_check(AST &ast, uint node, TypeCheckState &type_check_state)
{
	uint arguments_ed_idx = ast.data[node].sys_call.arguments_ed_idx;
	uint len              = ast.extra_data[arguments_ed_idx];
	for (uint i = arguments_ed_idx + 1; i < arguments_ed_idx + 1 + len; i++)
	{
		uint argument_node = ast.extra_data[i];
		ast_type_check(ast, argument_node, type_check_state);
	}
}

void
sys_call_code_gen(AST &ast, uint node, Assembler &assembler)
{
	uint arguments_ed_idx = ast.data[node].sys_call.arguments_ed_idx;
	uint len              = ast.extra_data[arguments_ed_idx];

	switch (ast.data[node].sys_call.sys_call_id)
	{
	case SysCallId::PRINT_CHAR:
	{
		if (len != 1)
		{
			err_at_token(ast.tokens[node], "Type Error",
				"Argument count in PRINT_CHAR SysCall is not equal to 1\n"
				"Expected a character as argument");
		}

		uint char_node = ast.extra_data[arguments_ed_idx + 1];

		if (ast.types[char_node].pointer_depth(ast.extra_data))
		{
			err_at_token(ast.tokens[node], "Type Error",
				"Argument in PRINT_CHAR SysCall is a pointer\n"
				"Expected a character as argument");
		}

		uint8_t char_reg = assembler.get_register();

		ast_get_value(ast, char_node, assembler, char_reg);
		assembler.print_char(char_reg);

		assembler.free_register(char_reg);

		break;
	}

	case SysCallId::GET_CHAR:
	{
		if (len != 1)
		{
			err_at_token(ast.tokens[node], "Type Error",
				"Argument count in GET_CHAR SysCall is not equal to 1\n"
				"Expected a pointer to a character as argument");
		}

		uint char_pointer_node = ast.extra_data[arguments_ed_idx + 1];

		if (ast.types[char_pointer_node].pointer_depth(ast.extra_data) != 1
			|| ast.types[char_pointer_node].pointed_type(ast.extra_data).byte_size(ast.extra_data) != 2)
		{
			err_at_token(ast.tokens[node], "Type Error",
				"Argument in GET_CHAR SysCall is a non-pointer or a pointer to a pointer\n"
				"Expected a pointer to a 16-bit character as argument");
		}

		uint8_t char_reg = assembler.get_register();
		assembler.get_char(char_reg);

		uint8_t addr_reg = assembler.get_register();
		ast_get_value(ast, char_pointer_node, assembler, addr_reg);
		assembler.store_ptr_16(char_reg, addr_reg);

		assembler.free_register(char_reg);
		assembler.free_register(addr_reg);

		break;
	}

	case SysCallId::UNKNOWN:
		err_at_token(ast.tokens[node], "Unknown SysCall",
			"Unknown SysCall with id %d", ast.data[node].sys_call.sys_call_id);
		break;
	}
}

#endif