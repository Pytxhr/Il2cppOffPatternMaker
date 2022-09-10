#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "IPrintable.h"

struct Reference;
struct Function;
struct cs_insn;

struct FunctionReferenceList : public IPrintable
{
private:
	std::unordered_map<Function*, std::vector<Reference>> RefsLists;
	std::mutex addMtx;

public:
	FunctionReferenceList();
	~FunctionReferenceList();

	void Print() override;
	void AddReference(Function* pFunc, cs_insn* pCurrInst, char accessMode = '\0');
	std::unordered_map<Function*, std::vector<Reference>>& getRefList();
	bool getRefsBySignature(const std::string& signature, std::pair<Function*, std::vector<Reference>>** outPairRefs) const;
};

