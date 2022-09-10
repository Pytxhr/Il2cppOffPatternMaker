#include "PatternEngine.h"
#include "Pattern.h"

bool PatternEngine::CompareMem(const unsigned char* memChunk, const unsigned char* mask, const size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		if (mask[i] != memChunk[i] && mask[i] != '?')
			return false;
	}

	return true;
}

/*void PatternEngine::FindPattern(const File* pFile, const Pattern& patternMask, std::vector<uintptr_t>& outResults)
{
	Buffer* pBuff = pFile->pBuff;
	const auto patternMaskBuff = patternMask.getBytes();

	if (pBuff) FindPattern(pBuff->mBuff, pBuff->mBuff + pBuff->mSize, patternMaskBuff.data(), patternMaskBuff.size(), outResults);
}*/

void PatternEngine::FindPattern(const unsigned char* pStartAddr, const unsigned char* pEndAddr, const unsigned char* pMask, size_t maskLen, std::vector<uintptr_t>& outResults)
{
	for (const unsigned char* pCurr = pStartAddr; (pCurr + maskLen) < pEndAddr; pCurr++)
	{
		if (CompareMem(pCurr, pMask, maskLen))
			outResults.push_back((uintptr_t)(pStartAddr + (pEndAddr - pCurr)));
	}
}
