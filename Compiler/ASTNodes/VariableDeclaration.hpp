#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"

struct VariableDeclaration final : public ASTNode
{
	std::unique_ptr<TypeIdentifierPair> type_and_id_pair;
	std::unique_ptr<ReadValue> assignment;
	IdentifierExpression id_expr;
	std::unique_ptr<ClassDefinition> class_definition;
	IdentifierKind id_kind;
	VariableDefinition variable_definition;

	VariableDeclaration(std::unique_ptr<TypeIdentifierPair> type_and_id_pair,
		std::unique_ptr<ReadValue> assignment)
		: ASTNode(type_and_id_pair->accountable_token, VARIABLE_DECLARATION),
		  type_and_id_pair(std::move(type_and_id_pair)),
		  assignment(std::move(assignment)),
		  id_expr(this->type_and_id_pair->accountable_token, this->type_and_id_pair->identifier) {}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		type_and_id_pair->dfs(callback, depth + 1);

		if (assignment)
		{
			assignment->dfs(callback, depth + 1);
		}

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s = "VariableDeclaration {} @ " + to_hex((size_t) this);
		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		type_and_id_pair->type_check(type_check_state);
		type = type_and_id_pair->type;

		std::string decl_name = type_and_id_pair->identifier;

		if (!type_check_state.add_var(decl_name, type))
		{
			err_at_token(accountable_token,
				"Duplicate identifier name",
				"Identifier %s is already declared",
				decl_name.c_str());
		}

		id_expr.type_check(type_check_state);

		if (!assignment)
		{
			return;
		}

		assignment->type_check(type_check_state);

		// Match types

		if (assignment->type.fits(type) == Type::Fits::NO)
		{
			warn("At %s, Initial value of VariableDeclaration does not"
			     "fit into specified type\n"
			     "lhs_type = %s, rhs_type = %s",
				assignment->accountable_token.to_str().c_str(),
				type.to_str().c_str(), assignment->type.to_str().c_str());
		}

		std::string id_name = type_and_id_pair->identifier;
		id_kind             = type_check_state.get_identifier_kind(id_name);

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
			variable_definition = type_check_state.get_local(id_name);
			break;

		case IdentifierKind::GLOBAL:
			variable_definition = type_check_state.globals[id_name];
			break;

		default:
			err_at_token(accountable_token, "Invalid VariableDeclaration",
				"Cannot declare a variable of this type\n"
				"Only locals and globals can be declared");
		}

		type = variable_definition.id.type;

		if (type.value >= BUILTIN_TYPE_END)
		{
			class_definition = std::make_unique<ClassDefinition>(
				type_check_state.classes[type.value]);
		}
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t init_value_reg;

		if (type.value > BUILTIN_TYPE_END)
		{
			return;
		}

		// Array declaration

		else if (type.is_array())
		{
			return;
		}

		// Primitive type declaration
		// Get the expression value into a register and store it in memory

		if (!assignment)
			return;

		init_value_reg = assembler.get_register();
		assignment->get_value(assembler, init_value_reg);
		id_expr.store(assembler, init_value_reg);
		assembler.free_register(init_value_reg);
	}
};

constexpr int VARIABLE_DECLARATION_SIZE = sizeof(VariableDeclaration);

#endif