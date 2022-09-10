#pragma once

#include <cstdint>

struct cs_insn;

class ArmCapstoneHelper{
public:
	static uint16_t GetLValueRegType(cs_insn* pInst);
	static uint16_t GetRValueRegType(cs_insn* pInst);
	static bool RegisterPresent(cs_insn* pInst, uint16_t reg);
	static bool HeuristicReturn(cs_insn* pInst);
};

class Arm64CapstoneHelper {
public:
	static uint16_t GetLValueRegType(cs_insn* pInst);
	static uint16_t GetRValueRegType(cs_insn* pInst);
	static bool RegisterPresent(cs_insn* pInst, uint16_t reg);
	static bool HeuristicReturn(cs_insn* pInst);
	static bool IsADRP(uintptr_t inst);
};


