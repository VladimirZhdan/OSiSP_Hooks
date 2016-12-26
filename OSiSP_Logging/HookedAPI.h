#pragma once



class HookedAPI
{
public:
	HookedAPI(PSTR moduleName, PSTR functionName, PROC hookedFunc);
	PROC basedFunc;
	~HookedAPI();
private: 
	PCSTR dllName, functionName;
	PROC hookedFunc;

	static void WINAPI ReplaceIATEntryInAllMods(PCSTR moduleName, PROC currentFunc, PROC newFunc);
	static void WINAPI ReplaceIATEntryInOneMod(HMODULE hMod, PCSTR moduleName, PROC currentFunc, PROC newFunc);
};

