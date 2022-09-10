#pragma once

#include "FunctionReferenceList.h"
#include "Pattern.h"
#include <vector>
#include "Function.h"
#include "LittleXrefs.h"

typedef std::pair<Function*, std::vector<Pattern>> PAIR_FUNC_PATTERNS;
typedef std::vector<PAIR_FUNC_PATTERNS> VEC_PAIR_FUNC_PATTERNS;

class Il2cppPatternMaker
{
public:
	static bool MakePatterns(LX::LittleXrefs* pOldLxRef, FunctionReferenceList& oldBinResult, LX::LittleXrefs* pNewLxRef, FunctionReferenceList& newBinResult, VEC_PAIR_FUNC_PATTERNS& outResult);
};

