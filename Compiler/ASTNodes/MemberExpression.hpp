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
	std::unique_ptr<IdentifierExpression> object;
	std::unique_ptr<IdentifierExpression> member;
	Operator op;
	std::unique_ptr<ClassDefinition> class_definition;

	MemberExpression(std::unique_ptr<IdentifierExpression> object,
		std::unique_ptr<IdentifierExpression> member, Token op_token)
		: WriteValue(std::move(op_token), MEMBER_EXPRESSION),
		  object(std::move(object)),
		  member(std::move(member)),
		  op(str_to_operator(accountable_token.value)) {}

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
		member->type_check(type_check_state);

		std::string member_name   = member->accountable_token.value;
		std::string instance_name = object->accountable_token.value;

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
			}

			err_at_token(accountable_token, "Member error",
				"Class %s has no member named %s",
				class_name.c_str(), member_name.c_str());
		}

		default:
			printf("operator %s in MemberExpression not implemented\n",
				op_to_str(op));
			abort();
			break;
		}

	gather_location_data:

		IdentifierKind id_kind = type_check_state.get_identifier_kind(instance_name);
		VariableDefinition var;

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
			var = type_check_state.locals[instance_name];
			break;

		case IdentifierKind::PARAMETER:
			var = type_check_state.parameters[instance_name];
			break;

		case IdentifierKind::GLOBAL:
			var = type_check_state.globals[instance_name];
			break;

		default:
			err_at_token(accountable_token, "Invalid MemberExpression",
				"Cannot access a member of this type\n"
				"Only members of locals and globals can be accessed");
		}

		const Type &member_type = class_definition->get_field_type(member_name);

		// Get the offset to the member

		int64_t offset;

		if (id_kind == IdentifierKind::PARAMETER)
		{
			offset = type_check_state.parameters_size - var.offset
				+ 8 + CPU::stack_frame_size;
		}
		else
		{
			offset = var.offset;
		}

		for (size_t i = 0; i < class_definition->fields.size(); i++)
		{
			if (class_definition->fields[i].name == member_name)
			{
				break;
			}

			offset += class_definition->fields[i].type.byte_size();
		}

		location_data = std::make_unique<LocationData>(
			id_kind, offset, member_type.byte_size());
	}

	void
	get_value(Assembler &assembler, uint8_t result_reg)
		const override
	{
		std::string instance_name = object->accountable_token.value;
		std::string member_name   = member->accountable_token.value;
		std::string class_name    = object->type.class_name;
		const Type &member_type   = class_definition->get_field_type(member_name);

		if (op == POINTER_TO_MEMBER)
		{
			if (object->type.pointer_depth() != 0)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s.%s syntax on a pointer\n"
					"Use %s->%s instead",
					instance_name.c_str(), member_name.c_str(),
					instance_name.c_str(), member_name.c_str());
			}

			// Local variable or parameter

			if (location_data->is_at_frame_top())
			{
				switch (location_data->var_size)
				{
				case 1:
					assembler.move_frame_offset_8_into_reg(
						location_data->offset, result_reg);
					break;

				case 2:
					assembler.move_frame_offset_16_into_reg(
						location_data->offset, result_reg);
					break;

				case 4:
					assembler.move_frame_offset_32_into_reg(
						location_data->offset, result_reg);
					break;

				case 8:
					assembler.move_frame_offset_64_into_reg(
						location_data->offset, result_reg);
					break;

				default:
					err_at_token(member->accountable_token,
						"Type Error",
						"Variable doesn't fit in register\n"
						"Support for this is not implemented yet");
				}

				return;
			}

			// Global variable

			switch (location_data->var_size)
			{
			case 1:
				assembler.move_stack_top_offset_8_into_reg(
					location_data->offset, result_reg);
				break;

			case 2:
				assembler.move_stack_top_offset_8_into_reg(
					location_data->offset, result_reg);
				break;

			case 4:
				assembler.move_stack_top_offset_8_into_reg(
					location_data->offset, result_reg);
				break;

			case 8:
				assembler.move_stack_top_offset_8_into_reg(
					location_data->offset, result_reg);
				break;

			default:
				err_at_token(member->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			return;
		}

		else if (op == DEREFERENCED_POINTER_TO_MEMBER)
		{
			if (object->type.pointer_depth() == 0)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s->%s syntax on an instance\n"
					"Use %s.%s instead",
					instance_name.c_str(), member_name.c_str(),
					instance_name.c_str(), member_name.c_str());
			}

			if (object->type.pointer_depth() != 1)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s->%s syntax on a pointer of depth %lu\n",
					instance_name.c_str(), member_name.c_str(),
					object->type.pointer_depth());
			}

			// Moves the pointer to the class into the result register

			object->get_value(assembler, result_reg);

			uint64_t offset = 0;

			for (size_t i = 0; i < class_definition->fields.size(); i++)
			{
				if (class_definition->fields[i].name == member_name)
				{
					break;
				}

				offset += class_definition->fields[i].type.byte_size();
			}

			// Adds the correct offset to the field to the pointer

			assembler.add_64_into_reg(offset, result_reg);

			// Dereference the value at the offset into the result register

			switch (member_type.byte_size())
			{
			case 1:
				assembler.move_reg_pointer_8_into_reg(
					result_reg, result_reg);
				break;

			case 2:
				assembler.move_reg_pointer_16_into_reg(
					result_reg, result_reg);
				break;

			case 4:
				assembler.move_reg_pointer_32_into_reg(
					result_reg, result_reg);
				break;

			case 8:
				assembler.move_reg_pointer_64_into_reg(
					result_reg, result_reg);
				break;

			default:
				err_at_token(member->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}
		}

		// Unknown operator

		else
		{
			err_at_token(accountable_token, "Syntax Error",
				"Unexpected token \"%s\" of type %s\n"
				"MemberExpressions only work with \".\" or \"->\" operators.",
				accountable_token.value.c_str(), token_type_to_str(accountable_token.type));
		}
	}

	void
	store(Assembler &assembler, uint8_t value_reg)
		const override
	{
		uint8_t this_ptr_reg;

		std::string instance_name = object->accountable_token.value;
		std::string member_name   = member->accountable_token.value;
		std::string class_name    = object->type.class_name;
		const Type &member_type   = class_definition->get_field_type(member_name);

		if (op == POINTER_TO_MEMBER)
		{
			if (object->type.pointer_depth() != 0)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s.%s syntax on a pointer\n"
					"Use %s->%s instead",
					instance_name.c_str(), member_name.c_str(),
					instance_name.c_str(), member_name.c_str());
			}

			// Local variable or parameter

			if (location_data->is_at_frame_top())
			{
				switch (location_data->var_size)
				{
				case 1:
					assembler.move_reg_into_frame_offset_8(
						value_reg, location_data->offset);
					break;

				case 2:
					assembler.move_reg_into_frame_offset_16(
						value_reg, location_data->offset);
					break;

				case 4:
					assembler.move_reg_into_frame_offset_32(
						value_reg, location_data->offset);
					break;

				case 8:
					assembler.move_reg_into_frame_offset_64(
						value_reg, location_data->offset);
					break;

				default:
					err_at_token(member->accountable_token,
						"Type Error",
						"Variable doesn't fit in register\n"
						"Support for this is not implemented yet");
				}

				return;
			}

			// Global variable

			switch (location_data->var_size)
			{
			case 1:
				assembler.move_reg_into_stack_top_offset_8(
					value_reg, location_data->offset);
				break;

			case 2:
				assembler.move_reg_into_stack_top_offset_16(
					value_reg, location_data->offset);
				break;

			case 4:
				assembler.move_reg_into_stack_top_offset_32(
					value_reg, location_data->offset);
				break;

			case 8:
				assembler.move_reg_into_stack_top_offset_64(
					value_reg, location_data->offset);
				break;

			default:
				err_at_token(member->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			return;
		}

		else if (op == DEREFERENCED_POINTER_TO_MEMBER)
		{
			if (object->type.pointer_depth() == 0)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s->%s syntax on an instance\n"
					"Use %s.%s instead",
					instance_name.c_str(), member_name.c_str(),
					instance_name.c_str(), member_name.c_str());
			}

			if (object->type.pointer_depth() != 1)
			{
				err_at_token(accountable_token, "Invalid MemberExpression",
					"Cannot use %s->%s syntax on a pointer of depth %lu\n",
					instance_name.c_str(), member_name.c_str(),
					object->type.pointer_depth());
			}

			// Moves the pointer to the class into the this ptr register

			this_ptr_reg = assembler.get_register();
			object->get_value(assembler, this_ptr_reg);

			uint64_t offset = 0;

			for (size_t i = 0; i < class_definition->fields.size(); i++)
			{
				if (class_definition->fields[i].name == member_name)
				{
					break;
				}

				offset += class_definition->fields[i].type.byte_size();
			}

			// Adds the correct offset to the field to the pointer

			assembler.add_64_into_reg(offset, this_ptr_reg);

			// Dereference the value at the offset into the this pointer register

			switch (member_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_reg_pointer_8(
					value_reg, this_ptr_reg);
				break;

			case 2:
				assembler.move_reg_into_reg_pointer_16(
					value_reg, this_ptr_reg);
				break;

			case 4:
				assembler.move_reg_into_reg_pointer_32(
					value_reg, this_ptr_reg);
				break;

			case 8:
				assembler.move_reg_into_reg_pointer_64(
					value_reg, this_ptr_reg);
				break;

			default:
				err_at_token(member->accountable_token,
					"Type Error",
					"Variable doesn't fit in register\n"
					"Support for this is not implemented yet");
			}

			assembler.free_register(this_ptr_reg);
		}

		// Unknown operator

		else
		{
			err_at_token(accountable_token, "Syntax Error",
				"Unexpected token \"%s\" of type %s\n"
				"MemberExpressions only work with \".\" or \"->\" operators.",
				accountable_token.value.c_str(), token_type_to_str(accountable_token.type));
		}
	}
};

#endif