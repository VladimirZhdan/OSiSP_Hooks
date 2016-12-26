#include "stdafx.h"
#include "HookedAPI.h"

#include <tlhelp32.h>
#include <ImageHlp.h>
#pragma comment(lib, "ImageHlp")

#include <fstream>

using namespace std;

HookedAPI::HookedAPI(PSTR moduleName, PSTR functionName, PROC hookedFunc)
{
	this->dllName = moduleName;
	this->functionName = functionName;
	this->hookedFunc = hookedFunc;

	this->basedFunc = GetProcAddress(GetModuleHandleA(moduleName), functionName);
	ReplaceIATEntryInAllMods(this->dllName, this->basedFunc, this->hookedFunc);
}

HookedAPI::~HookedAPI()
{
	ReplaceIATEntryInAllMods(this->dllName, this->hookedFunc, this->basedFunc);
}

static HMODULE ModuleFromAddress(PVOID pv)
{
	MEMORY_BASIC_INFORMATION mbi;
	return((VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
		? (HMODULE)mbi.AllocationBase : NULL);
}

void HookedAPI::ReplaceIATEntryInAllMods(PCSTR moduleName, PROC currentFunc, PROC newFunc)
{
	HMODULE hThisModule = ModuleFromAddress(ReplaceIATEntryInAllMods);
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	MODULEENTRY32 moduleEntry = { sizeof(moduleEntry) };

	for (BOOL bOk = Module32First(hSnapShot, &moduleEntry); bOk; bOk = Module32Next(hSnapShot, &moduleEntry))
	{
		if (moduleEntry.hModule != hThisModule)
		{
			ReplaceIATEntryInOneMod(moduleEntry.hModule, moduleName, currentFunc, newFunc);
		}
	}

}

void HookedAPI::ReplaceIATEntryInOneMod(HMODULE hMod, PCSTR moduleName, PROC currentFunc, PROC newFunc)
{
	ULONG ulSize;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = NULL;
	try
	{
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hMod, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
	}
	catch (...)
	{

	}

	if (pImportDesc == NULL)
	{
		return;
	}
	
	for (; pImportDesc->Name; pImportDesc++)
	{
		PSTR pszModName = (PSTR)((PBYTE) hMod + pImportDesc->Name);		
		if (lstrcmpiA(pszModName, moduleName) == 0)
		{			
			PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)((PBYTE)hMod + pImportDesc->FirstThunk);

			for (; pThunk->u1.Function; pThunk++)
			{				
				PROC *ppfn = (PROC*)&pThunk->u1.Function;
				BOOL bFound = (*ppfn == currentFunc);
				if (bFound)
				{						
					if (!WriteProcessMemory(GetCurrentProcess(), ppfn, &newFunc, sizeof(newFunc), NULL) && (GetLastError() == ERROR_NOACCESS))
					{
						DWORD dwOldProtect;
						if (VirtualProtect(ppfn, sizeof(newFunc), PAGE_WRITECOPY, &dwOldProtect))
						{
							WriteProcessMemory(GetCurrentProcess(), ppfn, &newFunc, sizeof(newFunc), NULL);
							VirtualProtect(ppfn, sizeof(newFunc), dwOldProtect, &dwOldProtect);							
						}
					}			
					return;					
				}
			}
		}
	}
}
