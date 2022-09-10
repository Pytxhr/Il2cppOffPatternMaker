#include "CapstoneManager.h"

CapstoneManager::CapstoneManager(const std::stack<csh>& _allHandles)
	: allHandles(_allHandles)
{}

void CapstoneManager::AdquireHandle(csh* pOutHandle)
{
	while (!TryAdquireHandle(pOutHandle)) std::this_thread::yield();
}

bool CapstoneManager::TryAdquireHandle(csh * pOutHandle)
{
	std::lock_guard<std::mutex> lck(OperationMtx);
	uintptr_t availableHandles = allHandles.size();

	if (availableHandles != 0)
	{
		*pOutHandle = allHandles.top();
		allHandles.pop();
		return true;
	}

	return false;
}

void CapstoneManager::ReleaseHandle(csh pOutHandle)
{
	std::lock_guard<std::mutex> lck(OperationMtx);

	allHandles.push(pOutHandle);
}

std::stack<csh>& CapstoneManager::getAllHandles()
{
	return allHandles;
}
