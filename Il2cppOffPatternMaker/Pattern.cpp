#include "Pattern.h"
#include "ByteHelper.h"

Pattern::Pattern()
	: cachedStr("")
{
}

void Pattern::CacheStr()
{
	const std::string qMark = std::string("?");

	for (size_t i = 0; i < aob.size(); i++)
	{
		const auto& pb = aob[i];

		cachedStr += i == 0 ? "" : " ";

		if (pb.byteType == ByteType::BYTE) cachedStr += ByteHelper::Byte2String(pb.byte);
		else cachedStr += qMark;
	}
}

void Pattern::InvalidateCache()
{
	cachedStr = "";
}

std::string Pattern::toString()
{
	if (cachedStr.empty()) CacheStr();

	return cachedStr;
}

void Pattern::SubPattern(size_t start, size_t end, Pattern& outPattern)
{
	outPattern.setAob(std::vector<PatternByte>(aob.begin() + start, aob.begin() + end));
}

void Pattern::AddByte(unsigned char byte, bool bIsWildCard)
{
	PatternByte pB;

	pB.byte = byte;
	pB.byteType = bIsWildCard ? ByteType::WILDCARD : ByteType::BYTE;

	aob.push_back(pB);

	InvalidateCache();
}

void Pattern::AddWildCard()
{
	AddByte(0x0, true);
}

void Pattern::setAob(const std::vector<PatternByte>& newAob)
{
	aob = newAob;
	InvalidateCache();
	CacheStr();
}

std::vector<unsigned char> Pattern::getBytes() const
{
	std::vector<unsigned char> bytes;

	for (const PatternByte& uc : aob)
		bytes.push_back(uc.byteType == ByteType::BYTE ? uc.byte : '?');

	return bytes;
}
