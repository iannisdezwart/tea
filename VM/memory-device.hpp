#ifndef TEA_MEMORY_DEVICE_HEADER
#define TEA_MEMORY_DEVICE_HEADER

#include <bits/stdc++.h>

using namespace std;

// Memory regions

#define IO_DEVICE_OFFSET 0x10
#define PROGRAM_START 0x100

class MemoryDevice {
	public:
		uint64_t from;
		uint64_t to;

		MemoryDevice(uint64_t from, uint64_t to) : from(from), to(to) {}

		virtual const char *type() = 0;
};

#endif