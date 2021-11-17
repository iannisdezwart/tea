#ifndef TEA_IO_DEVICE_HEADER
#define TEA_IO_DEVICE_HEADER

#include <bits/stdc++.h>

#include "memory-device.hpp"

using namespace std;

/**
 * @brief This class is not currently in use.
 * It was part of the memory mapper, but I removed it from the CPU class,
 * since it slowed down things considerably.
 */
class IODevice : public MemoryDevice {
	public:
		static constexpr const uint64_t size = 3;
		static constexpr const uint64_t stdin_read = 0;
		static constexpr const uint64_t stdout_write = 1;
		static constexpr const uint64_t stderr_write = 2;

		IODevice(uint64_t offset) : MemoryDevice(offset, offset + IODevice::size) {}

		const char *type()
		{
			return "IODevice";
		}

		template <typename intx_t>
		intx_t get(uint64_t offset)
		{
			switch (offset) {
				case stdin_read:
					return fgetc(stdin);

				default:
				{
					stringstream err_message;

					err_message << "Write-only memory access violation\n";
					err_message << "VM prevented reading from memory at 0x";
					err_message << hex << offset << '\n';

					throw err_message.str();
				}
			}
		}

		template <typename intx_t>
		void set(uint64_t offset, intx_t value)
		{
			switch (offset) {
				case stdout_write:
					putc(value, stdout);
					return;

				case stderr_write:
					putc(value, stderr);
					return;

				default:
				{
					stringstream err_message;

					err_message << "Read-only memory access violation\n";
					err_message << "VM prevented writing to memory at 0x";
					err_message << hex << offset << '\n';

					throw err_message.str();
				}
			}
		}
};

#endif