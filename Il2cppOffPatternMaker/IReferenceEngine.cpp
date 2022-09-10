#include "IReferenceEngine.h"
#include "FunctionReferenceList.h"
#include "Function.h"
#include "CapstoneHelper.h"

IReferenceEngine::IReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* _pCapstoneManager, uint16_t _abiRegBase)
	: pCapstoneManager(_pCapstoneManager)
	, pLx(_pLx)
	, abiRegBase(_abiRegBase)
{
}

void IReferenceEngine::DisasmCallback(Function* pFunc, std::function<void(cs_insn*, cs_insn*)> pCallBack)
{
	cs_insn* pDisasmdInst = nullptr;
	uintptr_t count = 0;
	csh capstoneHandle;

	pCapstoneManager->AdquireHandle(&capstoneHandle);
	count = cs_disasm(capstoneHandle, pFunc->entryMem, pFunc->pSeudoSize == 0 ? 0x1000 : pFunc->pSeudoSize, (uint64_t)(pFunc->entryMem), 0, &pDisasmdInst);
	pCapstoneManager->ReleaseHandle(capstoneHandle);

	if (count != 0)
	{		
		cs_insn* pDisasmdInstEnd = pDisasmdInst + count;

		pCallBack(pDisasmdInst, pDisasmdInstEnd);

		cs_free(pDisasmdInst, count);
	}
}

void IReferenceEngine::FindRefereces(Function* pFunc, const std::string& typeName, uint64_t offset, FunctionReferenceList& outRefsLists)
{
	std::vector<uint16_t> matchingTypenameParamsId;

	if (pFunc->getMatchingTypenameParamsId(typeName, matchingTypenameParamsId))
	{
		cs_insn* pDisasmdInst = nullptr;
		uintptr_t count = 0;

		DisasmCallback(pFunc, [&](cs_insn* pInstStart, cs_insn* pInstEnd) {
			cs_insn* pDisasmdInstEnd = pDisasmdInst + count;

			for (uint16_t currParamId : matchingTypenameParamsId)
			{
				OffsetAccessCallback(pInstStart, pInstEnd, abiRegBase + currParamId, offset, [&](cs_insn* pAccInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType) {
					outRefsLists.AddReference(pFunc, pAccInsStart, accessType);
				});
			}

			cs_free(pDisasmdInst, count);
		});
	}
}

void IReferenceEngine::FindReferences(Function* pFunc, uintptr_t offset, FunctionReferenceList& funcRefsLists)
{
	FindReferencesCallback(pFunc, offset, [&](cs_insn* pRefInsStart, uint16_t regType, cs_insn* pInsEnd, char accessType) {
		funcRefsLists.AddReference(pFunc, pRefInsStart, accessType);
	});
}


void IReferenceEngine::FindReferencesCallback(Function* pFunc, uintptr_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t regType, cs_insn* pInsEnd, char accessType)> pCallback) {
	DisasmCallback(pFunc, [&](cs_insn* pInstStart, cs_insn* pInstEnd) {

		FindReferencesCallback(pInstStart, pInstEnd, offset, pCallback);

		});
}

void IReferenceEngine::FindReferencesCallback(cs_insn* pStart, cs_insn* pEnd, uintptr_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t regType, cs_insn* pInsEnd, char accessType)> pCallback)
{
	uintptr_t assemblyBase = (uintptr_t)pLx->getAssemblyEntry();
	uintptr_t offPage = offset & ~0xFFF;
	uintptr_t offPageOff = offset - offPage;

	for (cs_insn* pCurrIns = pStart; pCurrIns < pEnd; pCurrIns++)
	{
		switch (pCurrIns->id)
		{

		case ARM64_INS_ADRP:
		{
			uintptr_t pagePointedCurrAdrp = uintptr_t(pCurrIns->detail->arm64.operands[1].imm - assemblyBase);
			uint16_t lreg = Arm64CapstoneHelper::GetLValueRegType(pCurrIns);

			if (pagePointedCurrAdrp == offPage)
			{
				OffsetAccessCallback(pCurrIns + 1, pEnd, lreg, offPageOff, [&](cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType) {
					pCallback(pRefInsStart, lreg, pEnd, accessType);
					});
			}
		} break;

		default:
			break;
		}
	}
}

void IReferenceEngine::FindReferencesCallback(Function* pFunc, uintptr_t offset, std::function<void(cs_insn* pRefIns, char accessType)> pCallback)
{
}

void IReferenceEngine::FindClazzReferences(Function* pFunc, uintptr_t offset, FunctionReferenceList& funcRefsLists)
{
	FindReferencesCallback(pFunc, offset, [&](cs_insn* pRefInsStart, uint16_t lregType, cs_insn* pInsEnd, char accessType) {
		funcRefsLists.AddReference(pFunc, pRefInsStart, accessType);
	});
}

void IReferenceEngine::FindClazzReferences(Function* pFunc, uintptr_t offset, uintptr_t clazzOffset, FunctionReferenceList& funcRefsLists)
{
	FindReferencesCallback(pFunc, offset, [&](cs_insn* pRefInsStart, uint16_t lregType, cs_insn* pInsEnd, char accessType) {
		if (accessType == 'r')
		{
			OffsetAccessCallback(pRefInsStart + 1, pInsEnd, lregType, 0, [&](cs_insn* pAccess2, uint16_t lRegType2, cs_insn* pInsEnd, char accessType2) {
				if (accessType == 'r')
				{
					OffsetAccessCallback(pAccess2 + 1, pInsEnd, lRegType2, getClazzIl2cppStaticOffset(), [&](cs_insn* pAccess3, uint16_t lRegType3, cs_insn* pInsEnd, char accessType3) {
						if (accessType == 'r')
						{
							OffsetAccessCallback(pAccess3 + 1, pInsEnd, lRegType3, clazzOffset, [&](cs_insn* pAccess4, uint16_t lRegType4, cs_insn* pInsEnd, char accessType4) {
								funcRefsLists.AddReference(pFunc, pAccess4, accessType4);
								});
						}
						});
				}
				});
		}
		});
}


void IReferenceEngine::FindReferences(Function* pFunc, uint16_t trackReg, uint64_t offset, FunctionReferenceList& outRefsLists)
{
	DisasmCallback(pFunc, [&](cs_insn* pInstStart, cs_insn* pInstEnd) {
		OffsetAccessCallback(pInstStart, pInstEnd, trackReg, offset, [&](cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType) {
			outRefsLists.AddReference(pFunc, pRefInsStart, accessType);
		});
	});
}

ArmReferenceEngine::ArmReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* pCapstoneManager)
	: IReferenceEngine(_pLx, pCapstoneManager, ARM_REG_R0)
{
}
//arm_reg::ARM_REG_R0

/*void ArmReferenceEngine::FindRefereces(Function* pFunc, cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, FunctionReferenceList& outRefsLists)
{
	for (cs_insn* pCurrInst = pStart; pCurrInst < pEnd; pCurrInst++)
	{
		if (ArmCapstoneHelper::HeuristicReturn(pCurrInst)) break;

		switch (pCurrInst->id)
		{
		case ARM_INS_LDR:
		case ARM_INS_LDRB:
		case ARM_INS_LDRBT:
		case ARM_INS_LDRD:
		{
			uint16_t lreg = ArmCapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = ArmCapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm.operands[1].mem.disp;

				if (disp == offset) // LDR? R?, [trackReg, #?? == offset?]
					outRefsLists.AddReference(pFunc, pCurrInst, 'r');

			} else if(lreg == trackReg) goto END_FIND;	// Register that contained instance of offset was overriden, 
									// no more work to do
			break;
		}

		case ARM_INS_STR:
		case ARM_INS_STRB:
		case ARM_INS_STRBT:
		case ARM_INS_STRD:
		{
			uint16_t rreg = ArmCapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm.operands[1].mem.disp;

				if (disp == offset) // LDR? R?, [trackReg, #?? == offset?]
					outRefsLists.AddReference(pFunc, pCurrInst, 'w');

			}

			break;
		}

		case ARM_INS_MOV:
		{
			uint16_t lreg = ArmCapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = ArmCapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg) FindRefereces(pFunc, pCurrInst + 1, pEnd, lreg, offset, outRefsLists);
			else if (lreg == trackReg) goto END_FIND;	// Again Register that contained instance of offset was overriden, 
									// no more work to do

			break;
		}


		}
	}
END_FIND:
	return;
}*/

Arm64ReferenceEngine::Arm64ReferenceEngine(LX::LittleXrefs* _pLx, CapstoneManager* pCapstoneManager)
	: IReferenceEngine(_pLx, pCapstoneManager, ARM64_REG_X0)
{

}
/*
void Arm64ReferenceEngine::FindRefereces(Function* pFunc, cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, FunctionReferenceList& outRefsLists)
{
	for (cs_insn* pCurrInst = pStart; pCurrInst < pEnd; pCurrInst++)
	{
		if (Arm64CapstoneHelper::HeuristicReturn(pCurrInst)) break;

		switch (pCurrInst->id)
		{

		case ARM64_INS_LDR:
		case ARM64_INS_LDRB:
		{
			uint16_t lreg = Arm64CapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm64.operands[1].mem.disp;

				if (disp == offset) // LDR? R?, [trackReg, #?? == offset?]
					outRefsLists.AddReference(pFunc, pCurrInst, 'r');

			}
			else if (lreg == trackReg) goto END_FIND;	// Register that contained instance of offset was overriden, 
								 // no more work to do
			break;
		}

		case ARM64_INS_STR:
		case ARM64_INS_STRB:
		{
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm64.operands[1].mem.disp;

				if (disp == offset) // LDR? X?, [trackReg, #?? == offset?]
					outRefsLists.AddReference(pFunc, pCurrInst, 'w');

			}

			break;
		}

		case ARM64_INS_MOV:
		{
			uint16_t lreg = Arm64CapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg) FindRefereces(pFunc, pCurrInst + 1, pEnd, lreg, offset, outRefsLists);
			else if (lreg == trackReg) goto END_FIND;	// Again Register that contained instance of offset was overriden, 
														// no more work to do

			break;
		}


		}
	}
END_FIND:
	return;
}
*/
void Arm64ReferenceEngine::OffsetAccessCallback(cs_insn* pStart, cs_insn* pEnd, uint16_t trackReg, uint64_t offset, std::function<void(cs_insn* pRefInsStart, uint16_t lRegType, cs_insn* pInsEnd, char accessType)> pOnFoundCallback)
{
	for (cs_insn* pCurrInst = pStart; pCurrInst < pEnd; pCurrInst++)
	{
		if (Arm64CapstoneHelper::HeuristicReturn(pCurrInst)) break;

		switch (pCurrInst->id)
		{

		case ARM64_INS_LDR:
		case ARM64_INS_LDRB:
		{
			uint16_t lreg = Arm64CapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm64.operands[1].mem.disp;

				if (disp == offset)
					pOnFoundCallback(pCurrInst, lreg, pEnd, 'r');
			}
			else if (lreg == trackReg) goto END_FIND;	// Register that contained instance of offset was overriden, 
								 // no more work to do
			break;
		}

		case ARM64_INS_STR:
		case ARM64_INS_STRB:
		{
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg)
			{
				uintptr_t disp = pCurrInst->detail->arm64.operands[1].mem.disp;

				if (disp == offset)
					pOnFoundCallback(pCurrInst, 0, pEnd, 'w');
			}

			break;
		}

		case ARM64_INS_MOV:
		{
			uint16_t lreg = Arm64CapstoneHelper::GetLValueRegType(pCurrInst);
			uint16_t rreg = Arm64CapstoneHelper::GetRValueRegType(pCurrInst);

			if (trackReg == rreg) OffsetAccessCallback(pCurrInst + 1, pEnd, lreg, offset, pOnFoundCallback);
			else if (lreg == trackReg) goto END_FIND;	// Again Register that contained instance of offset was overriden, 
														// no more work to do

			break;
		}

		case ARM64_INS_B:
		{
			auto branchingAddr = pCurrInst->detail->arm64.operands[0].imm;

			if ((pCurrInst->address - branchingAddr) < 0 && // Always Jumping Downwards, avoiding infinite loop
				IN_RANGE(int64_t(pStart->address), branchingAddr, int64_t(pEnd->address)))
			{
				uintptr_t instId = uintptr_t((branchingAddr - pStart->address) / 4);

				//printf("%x %s %s\n", 0, pCurrInst->mnemonic, pCurrInst->op_str);

				OffsetAccessCallback(pStart + instId, pEnd, trackReg, offset, pOnFoundCallback);
			}

			break;
		}


		}
	}
END_FIND:
	return;
}
