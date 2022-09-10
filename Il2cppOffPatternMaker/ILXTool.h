#pragma once

#include <vector>
#include <string>
#include <capstone\capstone.h>
#include "LittleXrefs.h"
#include "CapstoneManager.h"

using namespace LX;

struct FunctionReferenceList;
struct Function;
struct IReferenceEngine;

class ILXTool
{
private:
	CapstoneManager* pCapstoneManager;
	LittleXrefs* pLXRefs;
	std::vector<Function*> allFunctions;
	bool bAllFuncParsed;
protected:
	IReferenceEngine* RefsEngine;
public:
	ILXTool(LittleXrefs* _pLXRefs, cs_arch arch, cs_mode archMode);
	~ILXTool();

	void FindReferences(const std::string& className, uint64_t offset, FunctionReferenceList& ppOutReferenceList);
	void FindReferences(uint64_t offset, FunctionReferenceList& ppOutReferenceList);
	void FindClazzRefs(uint64_t offset, FunctionReferenceList& ppOutReferenceList);
	void FindClazzRefs(uint64_t offset, uint64_t clazzOff, FunctionReferenceList& ppOutReferenceList);
	LittleXrefs* getLittleXrefS();
	bool getFunctionAtOffset(uintptr_t offset, Function** pFunc);
	const std::vector<Function*>& getAllFunctions() const;
	void AddFunction(const std::string& name, const std::string& signature, uintptr_t offset, Function** pOutFunc = nullptr);
	CapstoneManager* GetCapstoneManager();
	void Run(int fileNumber, FunctionReferenceList& outfuncRefsLists);
	IReferenceEngine* getRefsEngine();
	void ParseAllFunction();
};

class LXARMTool : public ILXTool {
public:
	LXARMTool(LittleXrefs* pLXRefs);
	~LXARMTool();
};

class LXARM64Tool : public ILXTool {
public:
	LXARM64Tool(LittleXrefs* pLXRefs);
	~LXARM64Tool();
};


