// OSiSP_Logging.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include "HookedAPI.h"
#include <tchar.h>
#include <string>

typedef HANDLE(WINAPI * PCREATEFILEW) (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
typedef BOOL(WINAPI * PWRITEFILE)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
typedef BOOL(WINAPI * PREADFILE)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

typedef LONG(WINAPI * PREGOPENKEYEXW)(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
typedef LONG(WINAPI * PREGENUMKEYW)(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName);
typedef LONG(WINAPI * PREGENUMVALUEW)(HKEY hKey, DWORD dwIndex,	LPWSTR lpValueName, LPDWORD lpcchValueName,	LPDWORD lpReserved,	LPDWORD lpType,	LPBYTE lpData, LPDWORD lpcbData);
typedef LONG(WINAPI * PREGENUMKEYEXW)(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime);


HANDLE logFile;
DWORD t;
CRITICAL_SECTION cs;

extern HookedAPI HookedCreateFileW;
extern HookedAPI HookedWriteFile;
extern HookedAPI HookedReadFile;
extern HookedAPI HookedRegOpenKeyExW;
extern HookedAPI HookedRegEnumKeyW;
extern HookedAPI HookedRegEnumValueW;
extern HookedAPI HookedRegEnumKeyExW;

void LoggingFunc(LPCWSTR func, LPCWSTR param)
{
	EnterCriticalSection(&cs);

	WCHAR res[300] = L"";	
	wcscat_s(res, func);
	wcscat_s(res, L" ");
	wcscat_s(res, param);
	wcscat_s(res, L"\r\n");
	((PWRITEFILE)HookedWriteFile.basedFunc)(logFile, res, (_tcslen(res) * sizeof(WCHAR)), &t, NULL);

	LeaveCriticalSection(&cs);
}


HANDLE WINAPI Hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	HANDLE result = ((PCREATEFILEW)HookedCreateFileW.basedFunc)(lpFileName, dwDesiredAccess, dwShareMode,
		lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	LoggingFunc(L"CreateFileW", lpFileName);

	return result;
}

BOOL WINAPI Hook_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten,
	LPOVERLAPPED lpOverlapped)
{
	BOOL result = ((PWRITEFILE)HookedWriteFile.basedFunc)(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten,
		lpOverlapped);

	WCHAR tmp[20];
	_itow_s(*lpNumberOfBytesWritten, tmp, 10);
	wcscat_s(tmp, L" bytes");
	LoggingFunc(L"WriteFile", tmp);

	return result;
}

BOOL WINAPI Hook_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead,
	LPOVERLAPPED lpOverlapped)
{
	BOOL result = ((PREADFILE)HookedReadFile.basedFunc)(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead,
		lpOverlapped);

	WCHAR tmp[20];	
	_itow_s(*lpNumberOfBytesRead, tmp, 10);
	wcscat_s(tmp, L" bytes");
	LoggingFunc(L"ReadFile", tmp);

	return result;
}

LONG WINAPI Hook_RegOpenKeyExW(HKEY hKey, LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
	LONG result = ((PREGOPENKEYEXW)HookedRegOpenKeyExW.basedFunc)(hKey, lpSubKey, ulOptions, samDesired, phkResult);

	LoggingFunc(L"RegOpenKeyExW", lpSubKey);

	return result;
}

LONG WINAPI Hook_RegEnumKeyW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName)
{
	LONG result = ((PREGENUMKEYW)HookedRegEnumKeyW.basedFunc)(hKey, dwIndex, lpName, cchName);

	LoggingFunc(L"RegEnumKeyW", std::to_wstring(dwIndex).c_str());

	return result;
}

LONG WINAPI Hook_RegEnumValueW(HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) 
{
	LONG result = ((PREGENUMVALUEW)HookedRegEnumValueW.basedFunc)(hKey, dwIndex, lpValueName, lpcchValueName, lpReserved, lpType, lpData, lpcbData);	

	LoggingFunc(L"RegEnumValueW", std::to_wstring(dwIndex).c_str());

	return result;
}

LONG WINAPI Hook_RegEnumKeyExW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, PFILETIME lpftLastWriteTime)
{
	LONG result = ((PREGENUMKEYEXW)HookedRegEnumKeyExW.basedFunc)(hKey, dwIndex, lpName, lpcchName, lpReserved, lpClass, lpcchClass, lpftLastWriteTime);

	LoggingFunc(L"RegEnumKeyExW", std::to_wstring(dwIndex).c_str());

	return result;
}

HookedAPI HookedCreateFileW("Kernel32.dll", "CreateFileW", (PROC)Hook_CreateFileW);
HookedAPI HookedWriteFile("Kernel32.dll", "WriteFile", (PROC)Hook_WriteFile);
HookedAPI HookedReadFile("Kernel32.dll", "ReadFile", (PROC)Hook_ReadFile);
HookedAPI HookedRegOpenKeyExW("Advapi32.dll", "RegOpenKeyExW", (PROC)Hook_RegOpenKeyExW);
HookedAPI HookedRegEnumKeyW("Advapi32.dll", "RegEnumKeyW", (PROC)Hook_RegEnumKeyW);
HookedAPI HookedRegEnumValueW("Advapi32.dll", "RegEnumValueW", (PROC)Hook_RegEnumValueW);
HookedAPI HookedRegEnumKeyExW("Advapi32.dll", "RegEnumKeyExW", (PROC)Hook_RegEnumKeyExW);

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		InitializeCriticalSection(&cs);
		logFile = ((PCREATEFILEW)HookedCreateFileW.basedFunc)(L"E:\\LogFile.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, NULL, NULL);
				
		LoggingFunc(L"DLL_PROCESS_ATTACH", L"");
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:	
		break;
	case DLL_PROCESS_DETACH:
	{
		LoggingFunc(L"DLL_PROCESS_DETACH", L"");
	}
	break;
	}
	return TRUE;
}