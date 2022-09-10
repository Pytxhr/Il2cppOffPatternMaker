#include "ILXTool.h"
#include "Function.h"
#include "IReferenceEngine.h"
#include "FunctionReferenceList.h"
#include <iostream>
#include <conio.h>
#include <algorithm>
#include "ThreadPool.h"

void ILXTool::ParseAllFunction()
{
    if (bAllFuncParsed)
        return;

    auto& allFuncJson = getLittleXrefS()->getDumpJsonObj()["ScriptMethod"];
    Function* pBefore = nullptr;

    std::vector<const Json::Value*> allFuncJsonVec;

    for (size_t i = 0; i < allFuncJson.size(); i++)
    {
        const auto* pCurrFunc = &allFuncJson[i];

        if (pCurrFunc->isMember("Address"))
            allFuncJsonVec.push_back(pCurrFunc);
    }

    std::sort(allFuncJsonVec.begin(), allFuncJsonVec.end(), [](const Json::Value* e1, const Json::Value* e2) {
        const Json::Value& jsonVal1 = *e1;
        const Json::Value& jsonVal2 = *e2;

        return jsonVal1["Address"].asUInt64() < jsonVal2["Address"].asUInt64();
        });

    for (const auto* pCurrFunc : allFuncJsonVec)
    {
        const auto& currFunc = *pCurrFunc;
        Function* pCurr = nullptr;

        AddFunction(currFunc["Name"].asString(), currFunc["Signature"].asString(), currFunc["Address"].asUInt(), &pCurr);

        if (pBefore)
            pBefore->OnNextCreated(pCurr);

        pBefore = pCurr;
    }

    bAllFuncParsed = true;
}

LXARMTool::LXARMTool(LittleXrefs* pLXRefs)
    : ILXTool(pLXRefs, CS_ARCH_ARM, CS_MODE_ARM)
{
    RefsEngine = new ArmReferenceEngine(pLXRefs, GetCapstoneManager());
}

LXARMTool::~LXARMTool()
{
}

ILXTool::ILXTool(LittleXrefs* _pLXRefs, cs_arch arch, cs_mode archMode)
    : pLXRefs(_pLXRefs)
    , bAllFuncParsed(false)
{
    std::stack<csh> allHandles;

    //opening the handle of the capstone disasm
    
    for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
    {
        csh tmpHandle;

        if(cs_open(arch, archMode, &tmpHandle) != CS_ERR_OK)
            throw "Failed Opening Handle of Disassembler";

        // setting detailed mode on
        if (cs_option(tmpHandle, cs_opt_type::CS_OPT_DETAIL, true) != CS_ERR_OK)
        {
            cs_close(&tmpHandle);

            throw "Failed enabling Detailed Options";
        }

        allHandles.push(tmpHandle);
    }
    
    pCapstoneManager = new CapstoneManager(allHandles);
}

ILXTool::~ILXTool()
{
    for (Function* pFunc : allFunctions)
    {
        if (pFunc)
            delete pFunc;
    }

    if (pCapstoneManager)
    {
        auto allHandles = pCapstoneManager->getAllHandles();
        while (!allHandles.empty())
        {
            csh currHandle = allHandles.top(); allHandles.pop();

            cs_close(&currHandle);
        }

        delete pCapstoneManager;
    }
}

void ILXTool::FindReferences(const std::string& className, uint64_t offset, FunctionReferenceList& ppOutReferenceList)
{
    std::vector<Function*> candidatesFunctions;
    Function* pFunc = nullptr;

    for (auto* function : getAllFunctions())
    {
        if (function->ContainsParamType(className))
            candidatesFunctions.push_back(function);
    }

    if (candidatesFunctions.size() > 0)
    {
            ThreadPool pool;
        for (auto* candidateFunc : candidatesFunctions)
            pool.enqueue([&](Function* pFunc) {

                RefsEngine->FindRefereces(pFunc, className, offset, ppOutReferenceList);

                }, candidateFunc);
            
    }
}

void ILXTool::FindReferences(uint64_t offset, FunctionReferenceList& ppOutReferenceList)
{
    ThreadPool pool;
    for (auto* function : getAllFunctions())
        pool.enqueue([&](Function* pFunc) {
            RefsEngine->FindReferences(pFunc, (uintptr_t)offset, ppOutReferenceList);
            }, function);
}

void ILXTool::FindClazzRefs(uint64_t offset, FunctionReferenceList& ppOutReferenceList)
{
    ThreadPool pool;

    for (auto* function : getAllFunctions())
        pool.enqueue([&](Function* pFunc) {

        RefsEngine->FindClazzReferences(pFunc, (uintptr_t)offset, ppOutReferenceList);

            }, function);
        
}

void ILXTool::FindClazzRefs(uint64_t offset, uint64_t clazzOff, FunctionReferenceList& ppOutReferenceList)
{
    ThreadPool pool;

    for (auto* function : getAllFunctions())
    {
        /*if (function->signature == "void COW_GameFacade___cctor (const MethodInfo* method);")
            RefsEngine->FindClazzReferences(function, uintptr_t(offset), uintptr_t(clazzOff),ppOutReferenceList);*/

        pool.enqueue([&](Function* pFunc, uint64_t _offset, uint64_t _clazzOff, FunctionReferenceList* _ppOutReferenceList) {

            RefsEngine->FindClazzReferences(pFunc, uintptr_t(_offset), uintptr_t(_clazzOff), *_ppOutReferenceList);

            }, function, offset, clazzOff, &ppOutReferenceList);
    }
        
}

LittleXrefs* ILXTool::getLittleXrefS()
{
    return pLXRefs;
}

bool ILXTool::getFunctionAtOffset(uintptr_t offset, Function** pOutFunc)
{
    bool bFuncFound = false;

    for (auto* pFunc : getAllFunctions())
    {
        if ((bFuncFound = pFunc->OffsetAtBody(offset)))
        {
            if (pOutFunc)
                *pOutFunc = pFunc;

            break;
        }
    }

    return bFuncFound;
}

const std::vector<Function*>& ILXTool::getAllFunctions() const
{
    return allFunctions;
}

void ILXTool::AddFunction(const std::string& name, const std::string& signature, uintptr_t offset, Function** pOutFunc)
{
    Function* pFunc = new Function(name, signature, getLittleXrefS()->getAssemblyEntry() + offset, offset);

    allFunctions.push_back(pFunc);

    if (pOutFunc)
        *pOutFunc = pFunc;
}

CapstoneManager* ILXTool::GetCapstoneManager()
{
    return pCapstoneManager;
}

void ILXTool::Run(int fileNumber, FunctionReferenceList& outfuncRefsLists)
{
    system("cls");
    std::cout << "Information File " << fileNumber << std::endl << std::endl;
    std::string typeName = "";

    std::cout << "Input Type Name: "; std::cin >> typeName;
    uintptr_t offset = 0;
    uintptr_t clazz = 0;
    uintptr_t clazzOff = 0;

    if (typeName == "_clazz")
    {
        std::cout << "Input Clazz: "; std::cin >> std::hex >> clazz;

        FindClazzRefs(clazz, outfuncRefsLists);
    }
    else if(typeName == "_clazzoff") {
            
        std::cout << "Input Clazz: "; std::cin >> std::hex >> clazz;
        std::cout << "Input Clazz Offset: "; std::cin >> std::hex >> clazzOff;

        FindClazzRefs(clazz, clazzOff, outfuncRefsLists);
    }
    else {
        std::cout << "Input Offset: "; std::cin >> std::hex >> offset;

        FindReferences(typeName, offset, outfuncRefsLists);
    }
        
    system("cls");
}

IReferenceEngine* ILXTool::getRefsEngine()
{
    return RefsEngine;
}

LXARM64Tool::LXARM64Tool(LittleXrefs* pLXRefs)
    : ILXTool(pLXRefs, CS_ARCH_ARM64, CS_MODE_ARM)
{
    RefsEngine = new Arm64ReferenceEngine(pLXRefs, GetCapstoneManager());
}
