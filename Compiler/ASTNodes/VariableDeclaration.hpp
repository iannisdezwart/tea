#ifndef TEA_AST_NODE_VARIABLE_DECLARATION_HEADER
#define TEA_AST_NODE_VARIABLE_DECLARATION_HEADER

#include "Compiler/ASTNodes/ASTNode.hpp"
#include "Compiler/ASTNodes/ReadValue.hpp"
#include "Compiler/ASTNodes/IdentifierExpression.hpp"
#include "Compiler/tokeniser.hpp"
#include "Executable/byte-code.hpp"
#include "Compiler/util.hpp"
#include "Compiler/ASTNodes/TypeIdentifierPair.hpp"
#include "Compiler/ASTNodes/InitList.hpp"

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
		  id_expr(this->type_and_id_pair->accountable_token) {}

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

		std::string decl_name = type_and_id_pair->get_identifier_name();

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

		if (!assignment->type.fits(type))
		{
			err_at_token(assignment->accountable_token,
				"Type Error",
				"Initial value of VariableDeclaration does not fit into "
				"specified type\n"
				"type = %s, assignment_value = %s",
				type.to_str().c_str(), assignment->type.to_str().c_str());
		}

		std::string id_name = type_and_id_pair->get_identifier_name();
		id_kind             = type_check_state.get_identifier_kind(id_name);

		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
			variable_definition = type_check_state.locals[id_name];
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

		if (type == Type::USER_DEFINED_CLASS)
		{
			class_definition = std::make_unique<ClassDefinition>(
				type_check_state.classes[type.class_name]);
		}
	}

	void
	put_item_into_class_instance(Assembler &assembler, IdentifierKind id_kind,
		uint8_t init_list_item_reg, const Type &type, const Type &field_type,
		uint64_t offset, uint64_t &sub_offset, std::unique_ptr<ReadValue> &item)
		const
	{
		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
		{
			switch (field_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_frame_offset_8(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 1;
				break;

			case 2:
				assembler.move_reg_into_frame_offset_16(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 2;
				break;

			case 4:
				assembler.move_reg_into_frame_offset_32(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 4;
				break;

			case 8:
				assembler.move_reg_into_frame_offset_64(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 8;
				break;

			default:
			{
				uint8_t src_reg = assembler.get_register();
				assembler.move_reg_into_reg(R_FRAME_PTR, src_reg);
				assembler.add_64_into_reg(offset + sub_offset, src_reg);
				assembler.mem_copy_reg_pointer_8_to_reg_pointer_8(
					init_list_item_reg, src_reg, field_type.byte_size());
				break;
			}
			}

			break;
		}

		case IdentifierKind::GLOBAL:
		{
			switch (field_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_stack_top_offset_8(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 1;
				break;

			case 2:
				assembler.move_reg_into_stack_top_offset_16(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 2;
				break;

			case 4:
				assembler.move_reg_into_stack_top_offset_32(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 4;
				break;

			case 8:
				assembler.move_reg_into_stack_top_offset_64(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 8;
				break;

			default:
			{
				uint8_t src_reg = assembler.get_register();
				assembler.move_stack_top_address_into_reg(src_reg);
				assembler.add_64_into_reg(offset + sub_offset, src_reg);
				assembler.mem_copy_reg_pointer_8_to_reg_pointer_8(
					init_list_item_reg, src_reg, field_type.byte_size());
				break;
			}

			break;
			}
		}

		default:
		{
			err_at_token(item->accountable_token, "Internal Error",
				"Unknown identifier kind %d", id_kind);
		} // default
		} // switch
	}

	void
	put_item_into_array(Assembler &assembler, IdentifierKind id_kind,
		uint8_t init_list_item_reg, const Type &array_item_type,
		uint64_t offset, uint64_t &sub_offset, std::unique_ptr<ReadValue> &item)
		const
	{
		switch (id_kind)
		{
		case IdentifierKind::LOCAL:
		{
			switch (array_item_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_frame_offset_8(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 1;
				break;

			case 2:
				assembler.move_reg_into_frame_offset_16(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 2;
				break;

			case 4:
				assembler.move_reg_into_frame_offset_32(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 4;
				break;

			case 8:
				assembler.move_reg_into_frame_offset_64(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 8;
				break;

			default:
			{
				uint8_t src_reg = assembler.get_register();
				assembler.move_reg_into_reg(R_FRAME_PTR, src_reg);
				assembler.add_64_into_reg(offset + sub_offset, src_reg);
				assembler.mem_copy_reg_pointer_8_to_reg_pointer_8(
					init_list_item_reg, src_reg, array_item_type.byte_size());
				break;
			}
			}

			break;
		}

		case IdentifierKind::GLOBAL:
		{
			switch (array_item_type.byte_size())
			{
			case 1:
				assembler.move_reg_into_stack_top_offset_8(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 1;
				break;

			case 2:
				assembler.move_reg_into_stack_top_offset_16(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 2;
				break;

			case 4:
				assembler.move_reg_into_stack_top_offset_32(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 4;
				break;

			case 8:
				assembler.move_reg_into_stack_top_offset_64(
					init_list_item_reg, offset + sub_offset);
				sub_offset += 8;
				break;

			default:
			{
				uint8_t src_reg = assembler.get_register();
				assembler.move_stack_top_address_into_reg(src_reg);
				assembler.add_64_into_reg(offset + sub_offset, src_reg);
				assembler.mem_copy_reg_pointer_8_to_reg_pointer_8(
					init_list_item_reg, src_reg, array_item_type.byte_size());
				break;
			}
			}

			break;
		}

		default:
		{
			err_at_token(item->accountable_token, "Internal Error",
				"Unknown identifier kind %d", id_kind);
		} // default
		} // switch
	}

	void
	code_gen(Assembler &assembler)
		const override
	{
		uint8_t init_list_item_reg;
		uint8_t init_value_reg;

		// Class instance declaration

		if (type == Type::USER_DEFINED_CLASS)
		{
			if (!assignment)
			{
				// TODO: Implement default constructor
			}
			else if (assignment->node_type == INIT_LIST)
			{
				InitList *init_list = (InitList *) assignment.get();

				// Check type compatibility

				if (init_list->items.size() > class_definition->fields.size())
				{
					err_at_token(init_list->accountable_token, "Type Error",
						"InitList for %s class instance holds %lu members, "
						"but %s has only %lu fields",
						type.class_name.c_str(), init_list->items.size(),
						type.class_name.c_str(), class_definition->fields.size());
				}

				uint64_t sub_offset = 0;

				if (init_list->items.size())
					init_list_item_reg = assembler.get_register();

				for (size_t i = 0; i < init_list->items.size(); i++)
				{
					std::unique_ptr<ReadValue> &item = init_list->items[i];
					const Type item_type             = item->type;
					const Type &field_type           = class_definition->fields[i].type;

					// Type compatibility

					if (!item_type.fits(field_type))
					{
						// Todo: elaborate

						err_at_token(item->accountable_token, "Type Error",
							"Item %lu of InitList does not fit the corresponding field "
							"of class \"%s\"",
							i, type.class_name.c_str());
					}

					// Put item into class instance

					item->get_value(assembler, init_list_item_reg);
					put_item_into_class_instance(assembler,
						id_kind, init_list_item_reg, type, field_type,
						variable_definition.offset, sub_offset, item);
				}

				if (init_list->items.size())
					assembler.free_register(init_list_item_reg);
			}
			else
			{
				err_at_token(assignment->accountable_token, "Semantic Error",
					"Unexpected expression of type \"%s\" at the right hand "
					"side of a class instance declaration\n"
					"Expected an initialiser list or a constructor call",
					ast_node_type_to_str(assignment->node_type));
			}

			return;
		}

		// Array declaration

		else if (type.is_array())
		{
			if (!assignment)
				return;

			if (assignment->node_type != INIT_LIST)
			{
				err_at_token(assignment->accountable_token, "Semantic Error",
					"Expected an initialiser list or nothing on the right hand side "
					"of an array declaration\n"
					"Found an expression of type \"%s\"",
					ast_node_type_to_str(assignment->node_type));
			}

			InitList *init_list = (InitList *) assignment.get();

			// Check type compatibility

			if (init_list->items.size() > type.array_sizes.back())
			{
				err_at_token(init_list->accountable_token, "Type Error",
					"InitList for %s array holds %lu members, "
					"but the array only fits %lu elements",
					type.class_name.c_str(), init_list->items.size(),
					type.array_sizes.back());
			}

			Type array_item_type = type;
			array_item_type.array_sizes.pop_back();
			uint64_t sub_offset = 0;

			if (init_list->items.size())
				init_list_item_reg = assembler.get_register();

			for (size_t i = 0; i < init_list->items.size(); i++)
			{
				// Put item into array

				init_list->items[i]->get_value(assembler, init_list_item_reg);
				put_item_into_array(assembler, id_kind,
					init_list_item_reg, array_item_type,
					variable_definition.offset, sub_offset, init_list->items[i]);
			}

			if (init_list->items.size())
				assembler.free_register(init_list_item_reg);

			return;
		}

		// Built-in type declaration
		// Get the expression value into a register and store it in memory

		if (!assignment)
			return;

		init_value_reg = assembler.get_register();
		assignment->get_value(assembler, init_value_reg);
		id_expr.store(assembler, init_value_reg);
		assembler.free_register(init_value_reg);
	}
};

#endif