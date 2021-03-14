#ifndef TEA_MEMORY_MAPPER_HEADER
#define TEA_MEMORY_MAPPER_HEADER

#include <bits/stdc++.h>

#include "memory-device.hpp"
#include "ram-device.hpp"

using namespace std;

class MemoryMapper {
	public:
		vector<MemoryDevice *> devices;

		pair<size_t, size_t> find_device_index(uint64_t offset)
		{
			size_t low = 0;
			size_t high = devices.size();

			while (low != high) {
				size_t middle = (high - low) / 2 + low;

				if (offset >= devices[middle]->from && offset < devices[middle]->to) {
					return make_pair(middle, middle);
				} else if (offset < devices[middle]->from) {
					high = middle;
				} else {
					low = middle + 1;
				}
			}

			if (low == high) return make_pair(low, high + 1);
			return make_pair(low, high);
		}

		MemoryDevice *find_device(uint64_t offset)
		{
			pair<size_t, size_t> device_index = find_device_index(offset);

			if (device_index.first != device_index.second) {
				printf("Segmentation Fault\n");
				printf("VM prevented access to memory at %ld (0x%lx)\n",
					offset, offset);
				printf("No MemoryDevice is registered in this region\n");
				abort();
			}

			return devices[device_index.first];
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

			printf("Wasn't able to cast the MemoryDevice (get)\n");
			abort();
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			MemoryDevice *device = find_device(offset);
			uint64_t mapped_offset = offset - device->from;

			if (RamDevice *ram_device = dynamic_cast<RamDevice *>(device)) {
				return ram_device->set<intx_t>(mapped_offset, value);
			}

			printf("Wasn't able to cast the MemoryDevice (set)\n");
			abort();
		}

		void print()
		{
			for (size_t i = 0; i < devices.size(); i++) {
				printf("<Device> [ 0x%lx, 0x%lx ]\n", devices[i]->from, devices[i]->to);
			}
		}
};

#endif