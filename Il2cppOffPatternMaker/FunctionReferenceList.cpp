#include "FunctionReferenceList.h"
#include "Function.h"
#include "Reference.h"
#include <capstone/capstone.h>

FunctionReferenceList::FunctionReferenceList()
{
}

FunctionReferenceList::~FunctionReferenceList()
{
}

void FunctionReferenceList::Print()
{
	for (const auto& refList : RefsLists)
	{
		printf("%s:\n", refList.first->name.c_str());

		for (const auto& ref : refList.second)
			printf("    0x%lx %c:  %s\n", (long unsigned int)ref.RawOffset, ref.AccessMode, ref.instPreview.c_str());

		printf("\n");
	}
}

void FunctionReferenceList::AddReference(Function* pFunc, cs_insn* pCurrInst, char accessMode)
{
	std::lock_guard<std::mutex> lck(addMtx);

	Reference ref;

	ref.AccessMode = accessMode;

	if (ref.AccessMode == 0)
	{
		// TODO find access mode
	}

	ref.instPreview = std::string(pCurrInst->mnemonic) + " " + std::string(pCurrInst->op_str);
	ref.RawOffset = pFunc->entryOffset + (uintptr_t)(pCurrInst->address - (uintptr_t)pFunc->entryMem);

	if (RefsLists.find(pFunc) == RefsLists.end())
	{
		RefsLists[pFunc] = std::vector<Reference>();
		RefsLists[pFunc].reserve(5);
	}

	RefsLists[pFunc].push_back(ref);
}

std::unordered_map<Function*, std::vector<Reference>>& FunctionReferenceList::getRefList()
{
	return RefsLists;
}

bool FunctionReferenceList::getRefsBySignature(const std::string& signature, std::pair<Function*, std::vector<Reference>>** outPairRefs) const
{

	for (const auto& currKv : RefsLists)
	{
		if (!strcmp(currKv.first->signature.c_str(), signature.c_str()))
		{
			if (outPairRefs)
				*outPairRefs = (std::pair<Function*, std::vector<Reference>>*) & currKv;

			return true;
		}
				
	}

	return false;
}
