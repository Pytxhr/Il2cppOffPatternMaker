#pragma once

#include <capstone/capstone.h>
#include <stack>
#include <mutex>

class CapstoneManager
{
	std::stack<csh> allHandles;
	std::mutex OperationMtx;
public:
	CapstoneManager(const std::stack<csh>& _allHandles);

	void AdquireHandle(csh* pOutHandle);
	bool TryAdquireHandle(csh* pOutHandle);
	void ReleaseHandle(csh pOutHandle);

	std::stack<csh>& getAllHandles();
};

