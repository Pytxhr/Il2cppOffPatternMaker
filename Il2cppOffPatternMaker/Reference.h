#pragma once

#include <string>

struct Function;
struct cs_insn;

struct Reference
{
	std::string instPreview;
	char AccessMode; // R, W
	uintptr_t RawOffset;
	uintptr_t RvaOffset;
};

