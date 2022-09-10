#include "DiffHelper.h"
#include "Pattern.h"

/*void DiffHelper::MakeDiff(const File* pFile1, const File* pFile2, uint64_t off1, uint64_t off2, uint64_t diffSize, Pattern& outPatternResult)
{
    const auto* pFile1RawBuff = pFile1->pBuff->mBuff;
    const auto* pFile2RawBuff = pFile2->pBuff->mBuff;

    MakeDiff(pFile1RawBuff, pFile2RawBuff, off1, off2, diffSize, outPatternResult);
}*/

void DiffHelper::MakeDiff(const unsigned char* pBuff1, const unsigned char* pBuff2, uint64_t diffSize, Pattern& outPatternResult)
{
    for (size_t i = 0; i < diffSize; i++)
    {
        auto sample1Byte = pBuff1[i];

        if (sample1Byte == pBuff2[i]) outPatternResult.AddByte(sample1Byte, false);
        else  outPatternResult.AddWildCard();
    }

}
