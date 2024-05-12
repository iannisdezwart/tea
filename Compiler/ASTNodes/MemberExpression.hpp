#ifndef TEA_AST_NODES_MEMBER_EXPRESSION_HEADER
#define TEA_AST_NODES_MEMBER_EXPRESSION_HEADER

#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/WriteValue.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/code-gen/Assembler.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/tokeniser.hpp"
#include "VM/cpu.hpp"

struct MemberExpression final : public WriteValue
{
	std::unique_ptr<WriteValue> object;
	std::unique_ptr<IdentifierExpression> member;
	Operator op;
	std::unique_ptr<ClassDefinition> class_definition;
	bool is_ancestor;

	MemberExpression(std::unique_ptr<WriteValue> object,
		std::unique_ptr<IdentifierExpression> member, Token op_token)
		: WriteValue(std::move(op_token), MEMBER_EXPRESSION),
		  object(std::move(object)),
		  member(std::move(member)),
		  op(str_to_operator(accountable_token.value)),
		  is_ancestor(true)
	{
		if (this->object->node_type == MEMBER_EXPRESSION)
		{
			MemberExpression *member_expr = (MemberExpression *) this->object.get();
			member_expr->is_ancestor      = false;
		}
	}

	void
	dfs(std::function<void(ASTNode *, size_t)> callback, size_t depth)
		override
	{
		object->dfs(callback, depth + 1);
		member->dfs(callback, depth + 1);

		callback(this, depth);
	}

	std::string
	to_str()
		override
	{
		std::string s;

		s += "MemberExpression { op = \"";
		s += op_to_str(op);
		s += "\", object = \"";
		s += object->accountable_token.value;
		s += "\", member = \"";
		s += member->accountable_token.value;
		s += "\" } @ ";
		s += to_hex((size_t) this);

		return s;
	}

	void
	type_check(TypeCheckState &type_check_state)
		override
	{
		object->type_check(type_check_state);
		member->object_type.emplace(object->type);
		member->type_check(type_check_state);

		std::string instance_name = object->accountable_token.value;
		std::string member_name   = member->accountable_token.value;

		if (object->type != Type::USER_DEFINED_CLASS)
		{
			err_at_token(accountable_token, "Type Error",
				"Cannot access property %s from non-class type %s",
				member_name.c_str(),
				instance_name.c_str());
		}

		std::string class_name = object->type.class_name;
		class_definition       = std::make_unique<ClassDefinition>(
                        type_check_state.classes[class_name]);
		size_t offset = 0;

		if (op == POINTER_TO_MEMBER && object->type.pointer_depth() != 0)
		{
			err_at_token(accountable_token, "Invalid MemberExpression",
				"Cannot use %s.%s syntax on a pointer\n"
				"Use %s->%s instead",
				instance_name.c_str(), member_name.c_str(),
				instance_name.c_str(), member_name.c_str());
		}

		if (op == DEREFERENCED_POINTER_TO_MEMBER && object->type.pointer_depth() == 0)
		{
			err_at_token(accountable_token, "Invalid MemberExpression",
				"Cannot use %s->%s syntax on an instance\n"
				"Use %s.%s instead",
				instance_name.c_str(), member_name.c_str(),
				instance_name.c_str(), member_name.c_str());
		}

		if (op == DEREFERENCED_POINTER_TO_MEMBER && object->type.pointer_depth() != 1)
		{
			err_at_token(accountable_token, "Invalid MemberExpression",
				"Cannot use %s->%s syntax on a pointer of depth %lu\n",
				instance_name.c_str(), member_name.c_str(),
				object->type.pointer_depth());
		}

		switch (op)
		{
		case POINTER_TO_MEMBER:
		case DEREFERENCED_POINTER_TO_MEMBER:
		{
			// Find class in compiler state

			const ClassDefinition &cl = type_check_state.classes[class_name];

			for (size_t i = 0; i < cl.fields.size(); i++)
			{
				if (cl.fields[i].name == member_name)
				{
					type = cl.fields[i].type;
					goto gather_location_data;
				}

				offset += cl.fields[i].type.byte_size();
			}

			err_at_token(accountable_token, "Member error",
				"Class %s has no member named %s",
				class_name.c_str(), member_name.c_str());
		}

		default:
			err_at_token(accountable_token, "Syntax Error",
				"Unexpected token \"%s\" of type %s\n"
				"MemberExpressions only work with \".\" or \"->\" operators.",
				accountable_token.value.c_str(), token_type_to_str(accountable_token.type));
			abort();
			break;
		}

	gather_location_data:
		IdentifierKind id_kind = type != Type::USER_DEFINED_CLASS
			? object->location_data.id_kind
			: IdentifierKind::UNDEFINED;
		location_data          = LocationData(id_kind, offset);
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		// Get address of the class field

		object->get_value(assembler, result_reg);
		uint8_t offset_reg = assembler.get_register();
		assembler.move_lit(location_data.offset, offset_reg);
		assembler.add_int_64(offset_reg, result_reg);
		assembler.free_register(offset_reg);

		// Dereference

		if (!is_ancestor)
		{
			return;
		}

		switch (type.byte_size())
		{
		case 1:
			assembler.load_ptr_8(result_reg, result_reg);
			break;
		case 2:
			assembler.load_ptr_16(result_reg, result_reg);
			break;
		case 4:
			assembler.load_ptr_32(result_reg, result_reg);
			break;
		case 8:
			assembler.load_ptr_64(result_reg, result_reg);
			break;
		default:
			break;
		}
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		// Get address of the class field

		uint8_t dst_ptr_reg = assembler.get_register();

		object->get_value(assembler, dst_ptr_reg);
		uint8_t offset_reg = assembler.get_register();
		assembler.move_lit(location_data.offset, offset_reg);
		assembler.add_int_64(offset_reg, dst_ptr_reg);
		assembler.free_register(offset_reg);

		// Store value

		switch (type.byte_size())
		{
		case 1:
			assembler.store_ptr_8(value_reg, dst_ptr_reg);
			break;
		case 2:
			assembler.store_ptr_16(value_reg, dst_ptr_reg);
			break;
		case 4:
			assembler.store_ptr_32(value_reg, dst_ptr_reg);
			break;
		case 8:
			assembler.store_ptr_64(value_reg, dst_ptr_reg);
			break;
		default:
			assembler.mem_copy(value_reg, dst_ptr_reg, type.byte_size());
			break;
		}

		assembler.free_register(dst_ptr_reg);
	}
};

#endif