#ifndef TEA_MEMORY_MAPPER_HEADER
#define TEA_MEMORY_MAPPER_HEADER

#include <bits/stdc++.h>

#include "memory-device.hpp"
#include "ram-device.hpp"
#include "io-device.hpp"

/**
 * @brief This class is not currently in use.
 * It was part of the memory mapper, but I removed it from the CPU class,
 * since it slowed down things considerably.
 */
class MemoryMapper {
	public:
		std::vector<MemoryDevice *> devices;

		std::pair<size_t, size_t> find_device_index(uint64_t offset)
		{
			size_t low = 0;
			size_t high = devices.size();

			while (low != high) {
				size_t middle = (high - low) / 2 + low;

				if (offset >= devices[middle]->from && offset < devices[middle]->to) {
					return std::make_pair(middle, middle);
				} else if (offset < devices[middle]->from) {
					high = middle;
				} else {
					low = middle + 1;
				}
			}

			if (low == high) return std::make_pair(low, high + 1);
			return std::make_pair(low, high);
		}

		MemoryDevice *find_device(uint64_t offset)
		{
			std::pair<size_t, size_t> device_index = find_device_index(offset);

			if (device_index.first != device_index.second) {
					std::stringstream err_message;

					err_message << "Segmentation Fault\n";
					err_message << "VM prevented access to memory at 0x";
					err_message << std::hex << offset << '\n';
					err_message << "No MemoryDevice is registered in this region\n";

					throw err_message.str();
			}

			return devices[device_index.first];
		}

		bool is_safe(uint64_t offset)
		{
			std::pair<size_t, size_t> device_index = find_device_index(offset);
			return device_index.first == device_index.second;
		}

		void add_device(MemoryDevice *device)
		{
			if (devices.size() == 0) {
				devices.push_back(device);
				return;
			}

			size_t next_device_id = find_device_index(device->from).first;

			devices.push_back(NULL);

			// Shift devices after the insertion index to the right

			for (size_t i = devices.size() - 1; i > next_device_id; i--) {
				devices[i] = devices[i - 1];
			}

			devices[next_device_id] = device;
		}

		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			MemoryDevice *device = find_device(offset);
			uint64_t mapped_offset = offset - device->from;

			if (RamDevice *ram_device = dynamic_cast<RamDevice *>(device)) {
				return ram_device->get<intx_t>(mapped_offset);
			}

			if (IODevice *io_device = dynamic_cast<IODevice *>(device)) {
				return io_device->get<intx_t>(mapped_offset);
			}

			printf("Wasn't able to cast the MemoryDevice (get)\n"
				"I probably screwed up some code again ;-;\n");
			abort();
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			MemoryDevice *device = find_device(offset);
			uint64_t mapped_offset = offset - device->from;

			if (RamDevice *ram_device = dynamic_cast<RamDevice *>(device)) {
				ram_device->set<intx_t>(mapped_offset, value);
				return;
			}

			if (IODevice *io_device = dynamic_cast<IODevice *>(device)) {
				io_device->set<intx_t>(mapped_offset, value);
				return;
			}

			printf("Wasn't able to cast the MemoryDevice (set)\n"
				"I probably screwed up some code again ;-;\n");
			abort();
		}

		void print()
		{
			for (size_t i = 0; i < devices.size(); i++) {
				printf("<%s> [ 0x%lx, 0x%lx ]\n",
					devices[i]->type(), devices[i]->from, devices[i]->to);
			}
		}
};

class MemoryMapperReader {
	private:
		MemoryMapper& memory_mapper;

	public:
		size_t offset;

		MemoryMapperReader(MemoryMapper& memory_mapper, size_t offset)
			: memory_mapper(memory_mapper), offset(offset) {}

		bool is_safe()
		{
			return memory_mapper.is_safe(offset);
		}

		template <typename intx_t>
		intx_t read()
		{
			intx_t value = memory_mapper.get<intx_t>(offset);
			offset += sizeof(intx_t);
			return value;
		}
};

#endif