#pragma once

#include <vector>
#include <string>
#include "PatternByte.h"

struct Pattern
{

private:
	std::vector<PatternByte> aob;

public:
	Pattern();

	std::string cachedStr;

	void CacheStr();
	void InvalidateCache();
	std::string toString();
	void SubPattern(size_t start, size_t end, Pattern& outPattern);
	void AddByte(unsigned char byte, bool bIsWildCard = false);
	void AddWildCard();
	void setAob(const std::vector<PatternByte>& newAob);
	std::vector<unsigned char> getBytes() const;
};

