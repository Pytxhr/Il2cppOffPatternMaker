#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Parameter.h"

#define IN_RANGE(l, x, h) (((x) >= (l)) && ((x) < (h)))

#define TO_ANALIZE_MAX_FUNC_SZ 0x480

struct FunctionReferenceList;

struct Function
{
private:
	void ParseParameters();

public:
	Function(const std::string& name, const std::string& signature, unsigned char* currMemOff, uintptr_t offset);
	std::string name;
	std::string signature;
	uintptr_t entryOffset;
	unsigned char* entryMem;
	std::vector<Parameter> paramsList;
	uintptr_t pSeudoSize;

	bool ContainsParamType(std::string typeSignature);
	void FindReferences(const std::string& typeSignature, uint64_t offset, FunctionReferenceList& refsLists);
	bool getMatchingTypenameParamsId(const std::string& typeName, std::vector<uint16_t>& outParamsId);
	void OnNextCreated(Function* pNext);
	bool OffsetAtBody(uintptr_t offset);
	bool OffsetAtBody(unsigned char* offset);
	
};

