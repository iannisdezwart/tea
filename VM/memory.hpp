#ifndef TEA_MEMORY_HEADER
#define TEA_MEMORY_HEADER

#include <bits/stdc++.h>

#include "../Compiler/byte_code.hpp"

using namespace std;

class Memory {
	private:
		uint8_t *memory;

	public:
		size_t size;

		Memory(size_t mem_size)
		{
			size = mem_size;
			memory = new uint8_t[size];
		}

		Memory(uint8_t *init_mem, size_t init_mem_size)
		{
			size = init_mem_size;
			memory = new uint8_t[size];
			memcpy(memory, init_mem, init_mem_size);
		}

		~Memory()
		{
			delete[] memory;
		}

		uint8_t *location()
		{
			return memory;
		}

		void check_memory(uint8_t *location_p)
		{
			if (location_p < memory || location_p >= memory + size) {
				printf("Segmentation Fault\n");
				printf("VM prevented access to memory at %ld (0x%lx)\n",
					(size_t) location_p, (size_t) location_p);
				exit(139);
			}
		}

		template <typename intx_t>
		intx_t get(uint8_t *location_p)
		{
			check_memory(location_p);
			intx_t *value_p = (intx_t *) (location_p);
			return *value_p;
		}

		template <typename intx_t>
		void set(uint8_t *location_p, intx_t value)
		{
			check_memory(location_p);
			intx_t *value_p = (intx_t *) (location_p);
			*value_p = value;
		}

		void dump(
			uint8_t *left_bound,
			uint8_t *right_bound,
			uint8_t *highlight_fg = NULL,
			uint8_t *highlight_bg = NULL
		) {
			size_t i = 0;

			for (uint8_t *it = left_bound; it < right_bound; it++) {
				if (highlight_fg == it) printf("\x1b[31m");
				if (highlight_bg == it) printf("\x1b[43m");
				uint8_t byte = get<uint8_t>(it);
				printf("0x%04lx    0x%02hhx    %03hhu\x1b[m\n", i++, byte, byte);
			}

			printf("\n");
		}
};

class MemoryBuilder {
	public:
		vector<uint8_t> buffer;
		size_t i = 0;

		MemoryBuilder(size_t init_size = 1024) : buffer(init_size) {}

		template <typename intx_t>
		size_t push(intx_t value)
		{
			buffer.reserve(sizeof(intx_t));

			intx_t *value_p = (intx_t *) (buffer.data() + i);
			*value_p = value;

			size_t prev_i = i;
			i += sizeof(intx_t);
			return prev_i;
		}

		Memory *build(size_t stack_size)
		{
			return new Memory(buffer.data(), i + stack_size);
		}
};

class ProgramBuilder : public MemoryBuilder {
	public:
		unordered_map<string /* id */, uint64_t /* position */> labels;
		unordered_map<string /* id */, vector<uint64_t>> label_references;

		void push_instruction(enum Instruction instruction)
		{
			push<uint16_t>(instruction);
		}

		void push_address(uint8_t *address)
		{
			push((uint64_t) address);
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

		void move_8_into_mem(uint8_t lit, uint8_t *address)
		{
			push_instruction(MOVE_8_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_16_into_mem(uint16_t lit, uint8_t *address)
		{
			push_instruction(MOVE_16_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_32_into_mem(uint32_t lit, uint8_t *address)
		{
			push_instruction(MOVE_32_INTO_MEM);
			push(lit);
			push(address);
		}

		void move_64_into_mem(uint64_t lit, uint8_t *address)
		{
			push_instruction(MOVE_64_INTO_MEM);
			push(lit);
			push_address(address);
		}

		void move_reg_into_reg(uint8_t reg_id_1, uint8_t reg_id_2)
		{
			push_instruction(MOVE_REG_INTO_REG);
			push(reg_id_1);
			push(reg_id_2);
		}

		void move_reg_into_mem(uint8_t reg_id, uint8_t *address)
		{
			push_instruction(MOVE_REG_INTO_REG);
			push(reg_id);
			push_address(address);
		}

		void move_mem_into_reg(uint8_t *address, uint8_t reg_id)
		{
			push_instruction(MOVE_MEM_INTO_REG);
			push_address(address);
			push(reg_id);
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

		void move_reg_into_frame_offset(uint8_t reg_id, int64_t offset)
		{
			push_instruction(MOVE_REG_INTO_FRAME_OFFSET);
			push(reg_id);
			push(offset);
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

		void push_reg(uint8_t reg_id)
		{
			push_instruction(PUSH_REG);
			push(reg_id);
		}

		void pop_8_into_reg(uint8_t reg_id)
		{
			push_instruction(POP_8_INTO_REG);
			push(reg_id);
		}

		void pop_16_into_reg(uint16_t reg_id)
		{
			push_instruction(POP_16_INTO_REG);
			push(reg_id);
		}

		void pop_32_into_reg(uint32_t reg_id)
		{
			push_instruction(POP_32_INTO_REG);
			push(reg_id);
		}

		void pop_64_into_reg(uint64_t reg_id)
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

		void log_reg(uint8_t reg_id)
		{
			push_instruction(LOG_REG);
			push(reg_id);
		}

		void add_label(const string& id)
		{
			if (labels.count(id)) {
				printf("ProgramBuilder error: duplicate label %s\n", id.c_str());
				exit(1);
			}

			labels[id] = i;
		}

		void add_label_reference(const string& id)
		{
			label_references[id].push_back(i);
		}

		void update_label_references()
		{
			for (pair<const string&, vector<uint64_t>> ref : label_references) {
				const string& label = ref.first;
				vector<uint64_t>& reference_points = ref.second;

				// Get the location of the label

				if (!labels.count(label)) {
					printf("ProgramBuilder error: referenced non-defined label %s\n", label.c_str());
					exit(1);
				}

				const uint64_t& label_location = labels[label];

				// Update all label references

				for (size_t j = 0; j < reference_points.size(); j++) {
					uint64_t *reference_point = (uint64_t *) (buffer.data() + reference_points[j]);
					*reference_point = label_location;
				}
			}
		}

		void log_reg(uint8_t reg_id)
		{
			push_instruction(LOG_REG);
			push(reg_id);
		}
};

#endif