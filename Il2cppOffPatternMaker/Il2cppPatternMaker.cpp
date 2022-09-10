#include "Il2cppPatternMaker.h"
#include "Function.h"
#include "Reference.h"
#include <vector>
#include "DiffHelper.h"
#include "PatternEngine.h"
#include "ThreadPool.h"

#define MAX_PATTERN_SIZE 34
#define MAX_PATTERN_FOUNDS 3
#define PATTERN_SIZE_INCREMENT 1
#define PATTERN_SIZE_START 1

bool Il2cppPatternMaker::MakePatterns(LX::LittleXrefs* pOldLxRef, FunctionReferenceList& oldBinResult, LX::LittleXrefs* pNewLxRef, FunctionReferenceList& newBinResult, VEC_PAIR_FUNC_PATTERNS& outResult)
{
	bool result = false;

	auto& oldBinResultRefList = oldBinResult.getRefList();
	auto& newBinResultRefList = newBinResult.getRefList();

	std::vector< 
		std::pair< 
			std::pair< Function*, std::vector<Reference> >*, 
			std::pair< Function*, std::vector<Reference> >* 
		> 
	> mCandidates;

	for (auto& pCurrKV : oldBinResultRefList)
	{
		std::pair<Function*, std::vector<Reference>>* outCurrNewFuncRefPair = nullptr;

		if (newBinResult.getRefsBySignature(pCurrKV.first->signature, &outCurrNewFuncRefPair))
		{
			if (outCurrNewFuncRefPair->second.size() == pCurrKV.second.size())
				mCandidates.push_back(std::make_pair((std::pair< Function*, std::vector<Reference> >*) & pCurrKV, outCurrNewFuncRefPair));
		}
	}

	size_t foundPatterns = 0;

	ThreadPool tp;

	for (auto oldNewRefsPair : mCandidates)
	{
		if (foundPatterns >= MAX_PATTERN_FOUNDS)
			goto DONE;

		size_t refsCount = std::min(oldNewRefsPair.first->second.size(), oldNewRefsPair.second->second.size());
		PAIR_FUNC_PATTERNS currResult = std::make_pair(oldNewRefsPair.first->first, std::vector<Pattern>());

		for (size_t i = 0; i < refsCount && foundPatterns < MAX_PATTERN_FOUNDS; i++)
		{
			Reference& pOldRef = oldNewRefsPair.first->second[i];
			Reference& pNewRef = oldNewRefsPair.second->second[i];
			Pattern masterDiff;

			DiffHelper::MakeDiff(

				(const unsigned char*)(pOldLxRef->getAssemblyEntry() + pOldRef.RawOffset),
				(const unsigned char*)(pNewLxRef->getAssemblyEntry() + pNewRef.RawOffset),
				MAX_PATTERN_SIZE,
				masterDiff

			);			

			std::vector<unsigned char> masterDiffBytes = masterDiff.getBytes();

			for (int j = std::max(1, PATTERN_SIZE_START); j <= MAX_PATTERN_SIZE && foundPatterns < MAX_PATTERN_FOUNDS; j += PATTERN_SIZE_INCREMENT, j = std::min(j, MAX_PATTERN_SIZE))
			{
				if (masterDiffBytes[j] == '?')
					continue;

				std::vector<uintptr_t> pOldPatternResult;
				std::vector<uintptr_t> pNewPatternResult;

				PatternEngine::FindPattern(
					pOldLxRef->getAssemblyEntry(),
					pOldLxRef->getAssemblyEntry() + pOldLxRef->getAssemblySize(),
					masterDiffBytes.data(),
					j,
					pOldPatternResult
				);

				PatternEngine::FindPattern(
					pNewLxRef->getAssemblyEntry(),
					pNewLxRef->getAssemblyEntry() + pNewLxRef->getAssemblySize(),
					masterDiffBytes.data(),
					j,
					pNewPatternResult
				);

				bool bUnique = pOldPatternResult.size() == 1 && pNewPatternResult.size() == 1;
				
				if (bUnique)
				{
					Pattern toPushBack;

					masterDiff.SubPattern(0, j, toPushBack);

					currResult.second.push_back(toPushBack);
					foundPatterns++;
					break;
				}
			}
		}

		outResult.push_back(currResult);
	}

	DONE:

	return outResult.size() != 0;
}
