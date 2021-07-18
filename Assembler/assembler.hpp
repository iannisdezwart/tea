#ifndef TEA_ASSEMBLER_HEADER
#define TEA_ASSEMBLER_HEADER

#include <bits/stdc++.h>

#include "../VM/cpu.hpp"
#include "../VM/memory-device.hpp"
#include "byte_code.hpp"
#include "buffer-builder.hpp"
#include "buffer.hpp"

using namespace std;

struct StaticData {
	uint64_t offset;
	uint64_t size;

	uint64_t end()
	{
		return offset + size;
	}
};

class Assembler : public BufferBuilder {
	public:
		unordered_map<string /* id */, uint64_t /* position */> labels;
		unordered_map<string /* id */, vector<uint64_t>> label_references;
		vector<bool> free_registers;
		BufferBuilder static_data;

		Assembler() : free_registers(CPU::general_purpose_register_count, true) {}

		~Assembler()
		{
			free_buffer();
			static_data.free_buffer();
		}

		uint8_t get_register()
		{
			for (uint8_t i = 0; i < CPU::general_purpose_register_count; i++) {
				if (free_registers[i]) {
					free_registers[i] = false;
					return i;
				}
			}

			// Todo: create

			fprintf(stderr, "No free registers found. Spilling registers is not implemented yet.");
			abort();
		}

		void free_register(uint8_t reg_id)
		{
			free_registers[reg_id] = true;
		}

		Buffer assemble()
		{
			BufferBuilder executable;
			update_label_references();

			// Push the size of the static data segment
			// and the size of the program instructions

			executable.push<uint64_t>(static_data.offset);
			executable.push<uint64_t>(offset);

			// Combine static data and program instructions

			for (size_t j = 0; j < static_data.offset; j++) {
				executable.push(static_data[j]);
			}

			for (size_t j = 0; j < offset; j++) {
				executable.push(operator[](j));
			}

			// Create memory from the program and return it

			return executable.build();
		}

		void push_instruction(enum Instruction instruction)
		{
			push<uint16_t>(instruction);
		}

		void move_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(MOVE_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void move_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(MOVE_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void move_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(MOVE_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void move_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(MOVE_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void move_8_into_mem(uint8_t lit, uint64_t address)
		{
			push_instruction(MOVE_8_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_16_into_mem(uint16_t lit, uint64_t address)
		{
			push_instruction(MOVE_16_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_32_into_mem(uint32_t lit, uint64_t address)
		{
			push_instruction(MOVE_32_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_64_into_mem(uint64_t lit, uint64_t address)
		{
			push_instruction(MOVE_64_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_mem_8(uint8_t reg_id, uint64_t address)
		{
			push_instruction(MOVE_REG_INTO_MEM_8);
			push(reg_id);
			push(address);
		}

		void move_reg_into_mem_16(uint8_t reg_id, uint64_t address)
		{
			push_instruction(MOVE_REG_INTO_MEM_16);
			push(reg_id);
			push(address);
		}

		void move_reg_into_mem_32(uint8_t reg_id, uint64_t address)
		{
			push_instruction(MOVE_REG_INTO_MEM_32);
			push(reg_id);
			push(address);
		}

		void move_reg_into_mem_64(uint8_t reg_id, uint64_t address)
		{
			push_instruction(MOVE_REG_INTO_MEM_64);
			push(reg_id);
			push(address);
		}

		void move_mem_8_into_reg(uint64_t address, uint8_t reg_id)
		{
			push_instruction(MOVE_MEM_8_INTO_REG);
			push(address);
			push(reg_id);
		}

		void move_mem_16_into_reg(uint64_t address, uint8_t reg_id)
		{
			push_instruction(MOVE_MEM_16_INTO_REG);
			push(address);
			push(reg_id);
		}

		void move_mem_32_into_reg(uint64_t address, uint8_t reg_id)
		{
			push_instruction(MOVE_MEM_32_INTO_REG);
			push(address);
			push(reg_id);
		}

		void move_mem_64_into_reg(uint64_t address, uint8_t reg_id)
		{
			push_instruction(MOVE_MEM_64_INTO_REG);
			push(address);
			push(reg_id);
		}

		void move_reg_pointer_8_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_POINTER_8_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_pointer_16_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_POINTER_16_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_pointer_32_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_POINTER_32_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_pointer_64_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_POINTER_64_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_reg_pointer_8(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG_POINTER_8);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_reg_pointer_16(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG_POINTER_16);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_reg_pointer_32(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG_POINTER_32);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_reg_pointer_64(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG_POINTER_64);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_frame_offset_8_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_FRAME_OFFSET_8_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_frame_offset_16_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_FRAME_OFFSET_16_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_frame_offset_32_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_FRAME_OFFSET_32_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_frame_offset_64_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_FRAME_OFFSET_64_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_reg_into_frame_offset_8(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_FRAME_OFFSET_8);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_frame_offset_16(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_FRAME_OFFSET_32);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_frame_offset_32(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_FRAME_OFFSET_32);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_frame_offset_64(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_FRAME_OFFSET_64);
			push(reg_id);
			push(offset);
		}

		void move_stack_top_offset_8_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_STACK_TOP_OFFSET_8_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_stack_top_offset_16_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_STACK_TOP_OFFSET_16_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_stack_top_offset_32_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_STACK_TOP_OFFSET_32_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_stack_top_offset_64_into_reg(int64_t offset, uint8_t reg_id)
		{
			push_instruction(MOVE_STACK_TOP_OFFSET_64_INTO_REG);
			push(offset);
			push(reg_id);
		}

		void move_reg_into_stack_top_offset_8(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_8);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_stack_top_offset_16(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_32);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_stack_top_offset_32(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_32);
			push(reg_id);
			push(offset);
		}

		void move_reg_into_stack_top_offset_64(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_STACK_TOP_OFFSET_64);
			push(reg_id);
			push(offset);
		}

		void move_stack_top_address_into_reg(uint8_t reg_id)
		{
			push_instruction(MOVE_STACK_TOP_ADDRESS_INTO_REG);
			push(reg_id);
		}

		void add_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(ADD_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void add_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(ADD_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void add_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(ADD_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void add_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(ADD_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void add_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(ADD_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void subtract_8_from_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(SUBTRACT_8_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void subtract_16_from_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(SUBTRACT_16_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void subtract_32_from_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(SUBTRACT_32_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void subtract_64_from_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(SUBTRACT_64_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void subtract_reg_from_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(SUBTRACT_REG_FROM_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void multiply_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(MULTIPLY_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void multiply_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(MULTIPLY_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void multiply_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(MULTIPLY_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void multiply_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(MULTIPLY_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void multiply_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MULTIPLY_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void divide_8_from_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(DIVIDE_8_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void divide_16_from_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(DIVIDE_16_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void divide_32_from_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(DIVIDE_32_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void divide_64_from_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(DIVIDE_64_FROM_REG);
			push(lit);
			push(reg_id);
		}

		void divide_reg_from_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(DIVIDE_REG_FROM_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void take_modulo_8_of_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(TAKE_MODULO_8_OF_REG);
			push(lit);
			push(reg_id);
		}

		void take_modulo_16_of_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(TAKE_MODULO_16_OF_REG);
			push(lit);
			push(reg_id);
		}

		void take_modulo_32_of_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(TAKE_MODULO_32_OF_REG);
			push(lit);
			push(reg_id);
		}

		void take_modulo_64_of_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(TAKE_MODULO_64_OF_REG);
			push(lit);
			push(reg_id);
		}

		void take_modulo_reg_of_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(TAKE_MODULO_REG_OF_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void and_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(AND_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void and_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(AND_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void and_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(AND_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void and_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(AND_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void and_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(AND_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void or_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(OR_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void or_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(OR_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void or_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(OR_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void or_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(OR_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void or_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(OR_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void xor_8_into_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(XOR_8_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void xor_16_into_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(XOR_16_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void xor_32_into_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(XOR_32_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void xor_64_into_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(XOR_64_INTO_REG);
			push(lit);
			push(reg_id);
		}

		void xor_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(XOR_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void left_shift_reg_by_8(uint8_t reg_id, uint8_t shift_size)
		{
			push_instruction(LEFT_SHIFT_REG_BY_8);
			push(reg_id);
			push(shift_size);
		}

		void left_shift_reg_by_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(LEFT_SHIFT_REG_BY_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void right_shift_reg_by_8(uint8_t reg_id, uint8_t shift_size)
		{
			push_instruction(RIGHT_SHIFT_REG_BY_8);
			push(reg_id);
			push(shift_size);
		}

		void right_shift_reg_by_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(RIGHT_SHIFT_REG_BY_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void increment_reg(uint8_t reg_id)
		{
			push_instruction(INCREMENT_REG);
			push(reg_id);
		}

		void decrement_reg(uint8_t reg_id)
		{
			push_instruction(DECREMENT_REG);
			push(reg_id);
		}

		void not_reg(uint8_t reg_id)
		{
			push_instruction(NOT_REG);
			push(reg_id);
		}

		void compare_8_to_reg(uint8_t lit, uint8_t reg_id)
		{
			push_instruction(COMPARE_8_TO_REG);
			push(lit);
			push(reg_id);
		}

		void compare_16_to_reg(uint16_t lit, uint8_t reg_id)
		{
			push_instruction(COMPARE_16_TO_REG);
			push(lit);
			push(reg_id);
		}

		void compare_32_to_reg(uint32_t lit, uint8_t reg_id)
		{
			push_instruction(COMPARE_32_TO_REG);
			push(lit);
			push(reg_id);
		}

		void compare_64_to_reg(uint64_t lit, uint8_t reg_id)
		{
			push_instruction(COMPARE_64_TO_REG);
			push(lit);
			push(reg_id);
		}

		void compare_reg_to_8(uint8_t reg_id, uint8_t lit)
		{
			push_instruction(COMPARE_REG_TO_8);
			push(reg_id);
			push(lit);
		}

		void compare_reg_to_16(uint8_t reg_id, uint16_t lit)
		{
			push_instruction(COMPARE_REG_TO_16);
			push(reg_id);
			push(lit);
		}

		void compare_reg_to_32(uint8_t reg_id, uint32_t lit)
		{
			push_instruction(COMPARE_REG_TO_32);
			push(reg_id);
			push(lit);
		}

		void compare_reg_to_64(uint8_t reg_id, uint64_t lit)
		{
			push_instruction(COMPARE_REG_TO_64);
			push(reg_id);
			push(lit);
		}

		void compare_reg_to_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(COMPARE_REG_TO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void set_reg_if_greater(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_GREATER);
			push(reg_id);
		}

		void set_reg_if_greater_or_equal(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_GREATER_OR_EQUAL);
			push(reg_id);
		}

		void set_reg_if_less(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_LESS);
			push(reg_id);
		}

		void set_reg_if_less_or_equal(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_LESS_OR_EQUAL);
			push(reg_id);
		}

		void set_reg_if_equal(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_EQUAL);
			push(reg_id);
		}

		void set_reg_if_not_equal(uint8_t reg_id)
		{
			push_instruction(SET_REG_IF_NOT_EQUAL);
			push(reg_id);
		}

		void jump(const string& label)
		{
			push_instruction(JUMP);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_greater(const string& label)
		{
			push_instruction(JUMP_IF_GREATER);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_greater_or_equal(const string& label)
		{
			push_instruction(JUMP_IF_GREATER_OR_EQUAL);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_less(const string& label)
		{
			push_instruction(JUMP_IF_LESS);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_less_or_equal(const string& label)
		{
			push_instruction(JUMP_IF_LESS_OR_EQUAL);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_equal(const string& label)
		{
			push_instruction(JUMP_IF_EQUAL);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void jump_if_not_equal(const string& label)
		{
			push_instruction(JUMP_IF_NOT_EQUAL);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void push_8(uint8_t lit)
		{
			push_instruction(PUSH_8);
			push(lit);
		}

		void push_16(uint16_t lit)
		{
			push_instruction(PUSH_16);
			push(lit);
		}

		void push_32(uint32_t lit)
		{
			push_instruction(PUSH_32);
			push(lit);
		}

		void push_64(uint64_t lit)
		{
			push_instruction(PUSH_64);
			push(lit);
		}

		void push_reg_8(uint8_t reg_id)
		{
			push_instruction(PUSH_REG_8);
			push(reg_id);
		}

		void push_reg_16(uint8_t reg_id)
		{
			push_instruction(PUSH_REG_16);
			push(reg_id);
		}

		void push_reg_32(uint8_t reg_id)
		{
			push_instruction(PUSH_REG_32);
			push(reg_id);
		}

		void push_reg_64(uint8_t reg_id)
		{
			push_instruction(PUSH_REG_64);
			push(reg_id);
		}

		void pop_8_into_reg(uint8_t reg_id)
		{
			push_instruction(POP_8_INTO_REG);
			push(reg_id);
		}

		void pop_16_into_reg(uint8_t reg_id)
		{
			push_instruction(POP_16_INTO_REG);
			push(reg_id);
		}

		void pop_32_into_reg(uint8_t reg_id)
		{
			push_instruction(POP_32_INTO_REG);
			push(reg_id);
		}

		void pop_64_into_reg(uint8_t reg_id)
		{
			push_instruction(POP_64_INTO_REG);
			push(reg_id);
		}

		void call(const string& label)
		{
			push_instruction(CALL);
			add_label_reference(label);
			push<uint64_t>(0); // This will be updated later
		}

		void return_()
		{
			push_instruction(RETURN);
		}

		void allocate_stack(uint64_t size)
		{
			push_instruction(ALLOCATE_STACK);
			push(size);
		}

		void deallocate_stack(uint64_t size)
		{
			push_instruction(DEALLOCATE_STACK);
			push(size);
		}

		void log_reg(uint8_t reg_id)
		{
			push_instruction(LOG_REG);
			push(reg_id);
		}

		void comment(const string& str)
		{
			push_instruction(COMMENT);
			push_null_terminated_string(str);
		}

		void label(const string& str)
		{
			push_instruction(LABEL);
			push_null_terminated_string(str);
		}

		void add_label(const string& id)
		{
			if (labels.count(id)) {
				printf("ProgramBuilder error: duplicate label %s\n", id.c_str());
				abort();
			}

			labels[id] = offset;
		}

		void add_label_reference(const string& id)
		{
			label_references[id].push_back(offset);
		}

		void update_label_references()
		{
			for (pair<const string&, vector<uint64_t>> ref : label_references) {
				const string& label = ref.first;
				vector<uint64_t>& reference_points = ref.second;

				// Get the location of the label

				if (!labels.count(label)) {
					printf("ProgramBuilder error: referenced non-defined label %s\n",
						label.c_str());
					abort();
				}

				uint64_t label_location = PROGRAM_START + static_data.offset + labels[label];

				// Update all label references

				for (size_t j = 0; j < reference_points.size(); j++) {
					uint64_t *reference_point = (uint64_t *) (data() + reference_points[j]);
					*reference_point = label_location;
				}
			}
		}

		struct StaticData add_static_data(const uint8_t *data, size_t size)
		{
			size_t offset = PROGRAM_START + static_data.offset;

			for (size_t i = 0; i < size; i++) {
				static_data.push(data[i]);
			}

			return {
				.offset = offset,
				.size = size
			};
		}
};

#endif