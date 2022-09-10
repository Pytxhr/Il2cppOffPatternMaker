#pragma once

#include <cstdint>

struct File;
struct Pattern;

struct DiffHelper
{
	//static void MakeDiff(const File* pFile1, const File* pFile2, uint64_t off1, uint64_t off2, uint64_t diffSize, Pattern& outPatternResult);
	static void MakeDiff(const unsigned char* pBuff1, const unsigned char* pBuff2, uint64_t diffSize, Pattern& outPatternResult);
};

