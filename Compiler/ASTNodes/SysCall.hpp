#ifndef TEA_AST_NODE_SYS_CALL_HEADER_HEADER
#define TEA_AST_NODE_SYS_CALL_HEADER_HEADER

#include <set>
#include <string>

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"

std::set<std::string> syscall_names = { "PRINT_CHAR", "GET_CHAR" };

struct SysCall final : public ASTNode
{
	std::vector<std::unique_ptr<ReadValue>> arguments;

	SysCall(Token name_token, std::vector<std::unique_ptr<ReadValue>> &&arguments)
		: ASTNode(std::move(name_token), SYS_CALL),
		  arguments(std::move(arguments)) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		for (size_t i = 0; i < arguments.size(); i++)
		{
			arguments[i]->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "SysCall { name = \"" + accountable_token.value
			+ "\" } @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		for (size_t i = 0; i < arguments.size(); i++)
		{
			arguments[i]->type_check(type_check_state);
		}
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		if (accountable_token.value == "PRINT_CHAR")
		{
			if (arguments.size() != 1)
			{
				err_at_token(accountable_token, "Type Error",
					"Argument count in PRINT_CHAR SysCall is not equal to 1\n"
					"Expected a character as argument");
			}

			const std::unique_ptr<ReadValue> &character = arguments[0];
			const Type &type                            = character->type;

			if (type.pointer_depth())
			{
				err_at_token(accountable_token, "Type Error",
					"Argument in PRINT_CHAR SysCall is a pointer\n"
					"Expected a character as argument");
			}

			uint8_t char_reg = assembler.get_register();

			character->get_value(assembler, char_reg);
			assembler.print_char(char_reg);

			assembler.free_register(char_reg);
		}

		else if (accountable_token.value == "GET_CHAR")
		{
			if (arguments.size() != 1)
			{
				err_at_token(accountable_token, "Type Error",
					"Argument count in GET_CHAR SysCall is not equal to 1\n"
					"Expected a pointer to a character as argument");
			}

			const std::unique_ptr<ReadValue> &char_pointer = arguments[0];
			const Type &type                               = char_pointer->type;

			if (type.pointer_depth() != 1 || type.pointed_type().byte_size() != 2)
			{
				err_at_token(accountable_token, "Type Error",
					"Argument in GET_CHAR SysCall is a non-pointer or a pointer to a pointer\n"
					"Expected a pointer to a 16-bit character as argument");
			}

			uint8_t char_reg = assembler.get_register();
			assembler.get_char(char_reg);

			uint8_t addr_reg = assembler.get_register();
			char_pointer->get_value(assembler, addr_reg);
			assembler.store_ptr_16(char_reg, addr_reg);

			assembler.free_register(char_reg);
			assembler.free_register(addr_reg);
		}
	}
};

#endif