#include "Function.h"
#include <iostream>
#include <regex>

std::vector<std::string> cppSpecials{
    "static",
    "class",
    "const",
    "unsigned",
    "signed"
};

std::vector<std::string> toRemoveFromType{ 
    "*",
    "_c",
    "_o"
};

void Replace(std::string& str, const std::string& from, const std::string& to)
{
    while (str.find(from) != std::string::npos)
        str.replace(str.find(from), from.size(), to);
}

void RemoveKeywords(std::string& str, const std::vector<std::string>& keyworkdsToRemove)
{
    for (const std::string& keyword : keyworkdsToRemove)
        Replace(str, keyword, "");
}

Function::Function(const std::string& _name, const std::string& _signature, unsigned char* currMemOff, uintptr_t offset)
    : name(_name)
    , signature(_signature)
    , entryOffset(offset)
    , entryMem(currMemOff)
    , pSeudoSize(TO_ANALIZE_MAX_FUNC_SZ)
{
    ParseParameters();
}

bool Function::ContainsParamType(std::string typeSignature)
{
    for (const auto& param : paramsList)
        if (param.type == typeSignature) return true;

    return false;
}

void Function::FindReferences(const std::string& typeSignature, uint64_t offset, FunctionReferenceList& refsLists)
{

}

bool Function::getMatchingTypenameParamsId(const std::string& typeName, std::vector<uint16_t>& outParamsId)
{
    for (const auto& param : paramsList)
        if (param.type == typeName) outParamsId.push_back(param.id);

    return outParamsId.size() != 0;
}

void Function::OnNextCreated(Function* pNext)
{
    pSeudoSize = pNext->entryOffset - entryOffset;

    if (pSeudoSize > TO_ANALIZE_MAX_FUNC_SZ) 
        pSeudoSize = TO_ANALIZE_MAX_FUNC_SZ;
}

bool Function::OffsetAtBody(uintptr_t offset)
{
    return IN_RANGE(entryOffset, offset, entryOffset + pSeudoSize);
}

bool Function::OffsetAtBody(unsigned char* offset)
{
    return IN_RANGE(entryMem, offset, entryMem + pSeudoSize);;
}

void IgnoreTillChar(const char** pCurr, char stopAt, const char* limit = nullptr)
{
    while (**pCurr != stopAt && (*pCurr < limit || !limit))  (*pCurr)++;
}

void IgnoreIfChar(const char** pCurr,char c, const char* limit = nullptr)
{
    while (**pCurr == c && (*pCurr < limit || !limit))  (*pCurr)++;
}

void AddTillChar(const char** pCurr, std::string & adding, char stopAt, const char* limit = nullptr)
{
    while (**pCurr != stopAt && (*pCurr < limit || !limit))
    {
        adding += **pCurr;
        (*pCurr)++;
    }
}

void AddTillSpaces(const char** pCurr, std::string& adding, const char* limit = nullptr)
{
    AddTillChar(pCurr, adding, ' ', limit);
}

bool isCppSpecialKeyword(const std::string& keyword)
{
    for (const std::string& cppSpecial : cppSpecials)
    {
        if (cppSpecial == keyword)
            return true;
    }

    return false;
}

void Function::ParseParameters()
{
    const char* pSignatureStart = signature.c_str();
    const char* pSignatureEnd = pSignatureStart + signature.size();
    const char* pParamsParenthesesStart = pSignatureStart;

    while (*pParamsParenthesesStart != '(' && pParamsParenthesesStart < pSignatureEnd) pParamsParenthesesStart++;

    if (pParamsParenthesesStart != pSignatureEnd)
    {
        const char* pParamsParenthesesEnds = pParamsParenthesesStart;
        while (*pParamsParenthesesEnds != ')' && pParamsParenthesesEnds < pSignatureEnd) pParamsParenthesesEnds++;

        const char* currParamEntry = pParamsParenthesesStart + 1;

        while (currParamEntry < pParamsParenthesesEnds)
        {
            if (*currParamEntry == ' ') IgnoreIfChar(&currParamEntry,' ', pParamsParenthesesEnds);
            else {
                Parameter p;
                std::string parsed = "";
                bool bFirstKeyword = true;
                bool bIsCppKeyword = false;
                    
                do {
                    parsed = "";
                    AddTillSpaces(&currParamEntry, parsed, pParamsParenthesesEnds); currParamEntry++;
                    bIsCppKeyword = isCppSpecialKeyword(parsed);

                    if(!bIsCppKeyword && p.type.empty())
                        p.type += parsed;

                    p.signature += (bFirstKeyword ? "" : " ") + parsed;

                    bFirstKeyword = false;
                } while (bIsCppKeyword);
                
                IgnoreIfChar(&currParamEntry, ' ', pParamsParenthesesEnds);
                AddTillChar(&currParamEntry, p.name, ',', pParamsParenthesesEnds); currParamEntry++;

                p.signature += " " + p.name;

                RemoveKeywords(p.type, cppSpecials);
                RemoveKeywords(p.type, toRemoveFromType);

                paramsList.push_back(p);
                paramsList[paramsList.size() - 1].id = uint16_t(paramsList.size() - 1);
            }
        }
    }
}
