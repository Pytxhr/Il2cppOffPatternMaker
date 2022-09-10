#include <iostream>
#include "LittleXrefs.h"
#include "ILXTool.h"
#include "FunctionReferenceList.h"
#include "ThreadPool.h"
#include "Il2cppPatternMaker.h"
#include <conio.h>

int main()
{
    LX::LittleXrefs* pLxRefFile1;
    LX::LittleXrefs* pLxRefFile2;

    if (LX::MakeLittleXrefs(&pLxRefFile1) == LX::LX_OK && 
        LX::MakeLittleXrefs(&pLxRefFile2) == LX::LX_OK)
    {
        ILXTool* pTool1 = new LXARM64Tool(pLxRefFile1);
        ILXTool* pTool2 = new LXARM64Tool(pLxRefFile2);

        if (pTool1 && pTool2)
        {
            pTool1->ParseAllFunction();
            pTool2->ParseAllFunction();

            while (true)
            {
                FunctionReferenceList refsRes1;
                FunctionReferenceList refsRes2;

                pTool1->Run(0, refsRes1);
                pTool2->Run(1, refsRes2);

                VEC_PAIR_FUNC_PATTERNS finalResults;

                if (
                    Il2cppPatternMaker::MakePatterns(
                        pTool1->getLittleXrefS(),
                        refsRes1,
                        pTool2->getLittleXrefS(),
                        refsRes2,
                        finalResults
                    ))
                {
                    for (auto& pairFuncPatts : finalResults)
                    {
                        printf("%s:\n", pairFuncPatts.first->name.c_str());

                        for(Pattern& p : pairFuncPatts.second)
                            printf("   %s\n", p.cachedStr.c_str());

                        printf("\n");
                    }
                }
                else std::cout << "Unique Patterns Not Found!";

                _getch();
            }

            delete pTool1;
            delete pTool2;
        }

        delete pLxRefFile1;
        delete pLxRefFile2;
    }
}
