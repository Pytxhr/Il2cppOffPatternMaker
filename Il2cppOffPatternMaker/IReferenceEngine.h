#pragma once

#include <capstone/capstone.h>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <mutex>
#include "CapstoneManager.h"
#include "LittleXrefs.h"

struct Function;
struct FunctionReferenceList;

struct CrossReference {
	uintptr_t from;
	uintptr_t to;
};

struct IReferenceEngine
{
	uint16_t abiRegBase;
	CapstoneManager* pCapstoneManager;
	LX::LittleXrefs* pLx;

	IReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* pCapstoneManager, uint16_t abiRegBase);

	void FindReferences(Function* pFunc, uintptr_t offset, FunctionReferenceList& funcRefsLists);
	void FindReferences(Function* pFunc, uint16_t trackReg, uint64_t offset, FunctionReferenceList& outRefsLists);
	void FindRefereces(Function* pFunc, const std::string& typeName, uint64_t offset, FunctionReferenceList& outRefsLists);

	void FindReferencesCallback(Function* pFunc, uintptr_t offset, std::function<void(cs_insn* pRefIns, char accessType)> pCallback);
	void FindReferencesCallback(Function* pFunc, uintptr_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t regType, cs_insn* pInsEnd, char accessType)> pCallback);
	void FindReferencesCallback(cs_insn* pStart, cs_insn* pEnd, uintptr_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType)> pCallback);

	void FindClazzReferences(Function* pFunc, uintptr_t offset, FunctionReferenceList& funcRefsLists);
	void FindClazzReferences(Function* pFunc, uintptr_t offset,uintptr_t clazzOffset, FunctionReferenceList& funcRefsLists);
	

	void DisasmCallback(Function* pFunc, std::function<void(cs_insn* pInsStart, cs_insn* pInstEnd)> pCallBack);

	virtual void OffsetAccessCallback(cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType)>) = 0;

	virtual uintptr_t getClazzIl2cppStaticOffset() = 0;
};

struct ArmReferenceEngine : IReferenceEngine {
	ArmReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* pCapstoneManager);

	void OffsetAccessCallback(cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType)>) override {};

	uintptr_t getClazzIl2cppStaticOffset() override { return 0x5C; }
};

struct Arm64ReferenceEngine : IReferenceEngine {
	Arm64ReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* pCapstoneManager);

	void OffsetAccessCallback(cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType)> pOnFoundCallback) override;

	uintptr_t getClazzIl2cppStaticOffset() override { return 0xB8; }
};

