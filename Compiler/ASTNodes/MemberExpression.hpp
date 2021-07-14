#ifndef TEA_AST_NODES_MEMBER_EXPRESSION_HEADER
#define TEA_AST_NODES_MEMBER_EXPRESSION_HEADER

#include <bits/stdc++.h>

#include "../util.hpp"
#include "ASTNode.hpp"
#include "ReadValue.hpp"
#include "WriteValue.hpp"
#include "IdentifierExpression.hpp"
#include "../../Assembler/byte_code.hpp"
#include "../../Assembler/assembler.hpp"
#include "../compiler-state.hpp"
#include "../tokeniser.hpp"
#include "../../VM/cpu.hpp"

using namespace std;

class MemberExpression : public WriteValue {
	public:
		IdentifierExpression *object;
		IdentifierExpression *member;
		Token op_token;
		enum Operator op;

		MemberExpression(IdentifierExpression *object, IdentifierExpression *member,
			const Token& op_token)
				: object(object), member(member), op_token(op_token),
					op(str_to_operator(op_token.value)),
					WriteValue(op_token, MEMBER_EXPRESSION) {}

		~MemberExpression()
		{
			delete object;
			delete member;
		}

		void dfs(function<void(ASTNode *, size_t)> callback, size_t depth)
		{
			object->dfs(callback, depth + 1);
			member->dfs(callback, depth + 1);

			callback(this, depth);
		}

		string to_str()
		{
			string s;

			s += "MemberExpression { op = \"";
			s += op_to_str(op);
			s += "\", object = \"";
			s += object->identifier_token.value;
			s += "\", member = \"";
			s += member->identifier_token.value;
			s += "\" } @ ";
			s += to_hex((size_t) this);

			return s;
		}

		Type get_type(CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string member_name = member->identifier_token.value;

			if (object_type != Type::USER_DEFINED_CLASS) {
				err_at_token(op_token, "Type Error",
					"Cannot access property %s from non-class type %s",
					member_name.c_str(), object->identifier_token.value.c_str());
			}

			string class_name = object_type.class_name;

			switch (op) {
				case POINTER_TO_MEMBER:
				case DEREFERENCED_POINTER_TO_MEMBER:
				{
					// Find class in compiler state

					const Class& cl = compiler_state.classes[class_name];

					for (size_t i = 0; i < cl.fields.size(); i++) {
						if (cl.fields[i].name == member_name) {
							return cl.fields[i].type;
						}
					}

					err_at_token(op_token, "Member error",
						"Class %s has no member named %s",
						class_name.c_str(), member_name.c_str());
				}

				default:
					printf("operator %s in MemberExpression not implemented\n", op_to_str(op));
					abort();
					break;
			}
		}

		LocationData get_location_data(CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string instance_name = object->identifier_token.value;
			string member_name = member->identifier_token.value;

			IdentifierKind id_kind = compiler_state.get_identifier_kind(instance_name);

			Variable var;

			switch (id_kind) {
				case IdentifierKind::LOCAL:
					var = compiler_state.locals[instance_name];
					break;

				case IdentifierKind::PARAMETER:
					var = compiler_state.parameters[instance_name];
					break;

				case IdentifierKind::GLOBAL:
					var = compiler_state.globals[instance_name];
					break;

				default:
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot access a member of this type\n"
						"Only members of locals and globals can be accessed");
			}

			string class_name = object_type.class_name;
			const Class& cl = compiler_state.classes[class_name];
			const Type& member_type = cl.get_field_type(member_name);

			// Get the offset to the member

			int64_t offset;

			if (id_kind == IdentifierKind::PARAMETER) {
				offset = compiler_state.parameters_size - var.offset
					+ 8 + CPU::stack_frame_size;
			} else {
				offset = var.offset;
			}

			for (size_t i = 0; i < cl.fields.size(); i++) {
				if (cl.fields[i].name == member_name) {
					break;
				}

				offset += cl.fields[i].type.byte_size();
			}

			return LocationData(id_kind, offset, member_type.byte_size());
		}

		void get_value(Assembler& assembler, CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string instance_name = object->identifier_token.value;
			string member_name = member->identifier_token.value;
			string class_name = object_type.class_name;
			const Class& cl = compiler_state.classes[class_name];
			const Type& member_type = cl.get_field_type(member_name);

			LocationData location_data = get_location_data(compiler_state);

			if (op == POINTER_TO_MEMBER) {
				if (object_type.pointer_depth() != 0) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s.%s syntax on a pointer\n"
						"Use %s->%s instead",
						instance_name.c_str(), member_name.c_str(),
						instance_name.c_str(), member_name.c_str());
				}

				// Local variable or parameter

				if (location_data.is_at_frame_top()) {
					switch (location_data.var_size) {
						case 1:
							assembler.move_frame_offset_8_into_reg(
								location_data.offset, R_ACCUMULATOR_0_ID);
							break;

						case 2:
							assembler.move_frame_offset_16_into_reg(
								location_data.offset, R_ACCUMULATOR_0_ID);
							break;

						case 4:
							assembler.move_frame_offset_32_into_reg(
								location_data.offset, R_ACCUMULATOR_0_ID);
							break;

						case 8:
							assembler.move_frame_offset_64_into_reg(
								location_data.offset, R_ACCUMULATOR_0_ID);
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

				switch (location_data.var_size) {
					case 1:
						assembler.move_stack_top_offset_8_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 2:
						assembler.move_stack_top_offset_8_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 4:
						assembler.move_stack_top_offset_8_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					case 8:
						assembler.move_stack_top_offset_8_into_reg(
							location_data.offset, R_ACCUMULATOR_0_ID);
						break;

					default:
						err_at_token(member->accountable_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}

				return;
			}

			else if (op == DEREFERENCED_POINTER_TO_MEMBER) {
				if (object_type.pointer_depth() == 0) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s->%s syntax on an instance\n"
						"Use %s.%s instead",
						instance_name.c_str(), member_name.c_str(),
						instance_name.c_str(), member_name.c_str());
				}

				if (object_type.pointer_depth() != 1) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s->%s syntax on a pointer of depth %lu\n",
						instance_name.c_str(), member_name.c_str(), object_type.pointer_depth());
				}

				// Moves the pointer to the class into R_ACCUMULATOR_0_ID

				object->get_value(assembler, compiler_state);

				uint64_t offset = 0;

				for (size_t i = 0; i < cl.fields.size(); i++) {
					if (cl.fields[i].name == member_name) {
						break;
					}

					offset += cl.fields[i].type.byte_size();
				}

				// Adds the correct offset to the field to the pointer

				assembler.add_64_into_reg(offset, R_ACCUMULATOR_0_ID);

				// Dereference the value at the offset into R_ACCUMULATOR_0

				switch (member_type.byte_size()) {
					case 1:
						assembler.move_reg_pointer_8_into_reg(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
						break;

					case 2:
						assembler.move_reg_pointer_16_into_reg(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
						break;

					case 4:
						assembler.move_reg_pointer_32_into_reg(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
						break;

					case 8:
						assembler.move_reg_pointer_64_into_reg(
							R_ACCUMULATOR_0_ID, R_ACCUMULATOR_0_ID);
						break;

					default:
						err_at_token(member->accountable_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}
			}

			// Unknown operator

			else {
				err_at_token(op_token, "Syntax Error",
					"Unexpected token \"%s\" of type %s\n"
					"MemberExpressions only work with \".\" or \"->\" operators.",
					op_token.value.c_str(), token_type_to_str(op_token.type));
			}
		}

		void store(Assembler& assembler, CompilerState& compiler_state)
		{
			Type object_type = object->get_type(compiler_state);
			string instance_name = object->identifier_token.value;
			string member_name = member->identifier_token.value;
			string class_name = object_type.class_name;
			const Class& cl = compiler_state.classes[class_name];
			const Type& member_type = cl.get_field_type(member_name);

			LocationData location_data = get_location_data(compiler_state);

			if (op == POINTER_TO_MEMBER) {
				if (object_type.pointer_depth() != 0) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s.%s syntax on a pointer\n"
						"Use %s->%s instead",
						instance_name.c_str(), member_name.c_str(),
						instance_name.c_str(), member_name.c_str());
				}

				// Local variable or parameter

				if (location_data.is_at_frame_top()) {
					switch (location_data.var_size) {
						case 1:
							assembler.move_reg_into_frame_offset_8(
								R_ACCUMULATOR_0_ID, location_data.offset);
							break;

						case 2:
							assembler.move_reg_into_frame_offset_16(
								R_ACCUMULATOR_0_ID, location_data.offset);
							break;

						case 4:
							assembler.move_reg_into_frame_offset_32(
								R_ACCUMULATOR_0_ID, location_data.offset);
							break;

						case 8:
							assembler.move_reg_into_frame_offset_64(
								R_ACCUMULATOR_0_ID, location_data.offset);
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

				switch (location_data.var_size) {
					case 1:
						assembler.move_reg_into_stack_top_offset_8(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 2:
						assembler.move_reg_into_stack_top_offset_16(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 4:
						assembler.move_reg_into_stack_top_offset_32(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					case 8:
						assembler.move_reg_into_stack_top_offset_64(
							R_ACCUMULATOR_0_ID, location_data.offset);
						break;

					default:
						err_at_token(member->accountable_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}

				return;
			}

			else if (op == DEREFERENCED_POINTER_TO_MEMBER) {
				if (object_type.pointer_depth() == 0) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s->%s syntax on an instance\n"
						"Use %s.%s instead",
						instance_name.c_str(), member_name.c_str(),
						instance_name.c_str(), member_name.c_str());
				}

				if (object_type.pointer_depth() != 1) {
					err_at_token(op_token, "Invalid MemberExpression",
						"Cannot use %s->%s syntax on a pointer of depth %lu\n",
						instance_name.c_str(), member_name.c_str(), object_type.pointer_depth());
				}

				// Moves the value to store into R_ACCUMULATOR_1_ID

				assembler.move_reg_into_reg(R_ACCUMULATOR_0_ID, R_ACCUMULATOR_1_ID);

				// Moves the pointer to the class into R_ACCUMULATOR_0_ID

				object->get_value(assembler, compiler_state);

				uint64_t offset = 0;

				for (size_t i = 0; i < cl.fields.size(); i++) {
					if (cl.fields[i].name == member_name) {
						break;
					}

					offset += cl.fields[i].type.byte_size();
				}

				// Adds the correct offset to the field to the pointer

				assembler.add_64_into_reg(offset, R_ACCUMULATOR_0_ID);

				// Dereference the value at the offset into R_ACCUMULATOR_0

				switch (member_type.byte_size()) {
					case 1:
						assembler.move_reg_into_reg_pointer_8(
							R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
						break;

					case 2:
						assembler.move_reg_into_reg_pointer_16(
							R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
						break;

					case 4:
						assembler.move_reg_into_reg_pointer_32(
							R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
						break;

					case 8:
						assembler.move_reg_into_reg_pointer_64(
							R_ACCUMULATOR_1_ID, R_ACCUMULATOR_0_ID);
						break;

					default:
						err_at_token(member->accountable_token,
							"Type Error",
							"Variable doesn't fit in register\n"
							"Support for this is not implemented yet");
				}
			}

			// Unknown operator

			else {
				err_at_token(op_token, "Syntax Error",
					"Unexpected token \"%s\" of type %s\n"
					"MemberExpressions only work with \".\" or \"->\" operators.",
					op_token.value.c_str(), token_type_to_str(op_token.type));
			}
		}
};

#endif