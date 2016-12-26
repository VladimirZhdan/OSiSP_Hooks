// OSiSP_Hooks.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream>

BOOL WINAPI LoadDllToProcessUsingRemoteThread(HANDLE hProcess, PCWSTR dllName)
{
	BOOL result = FALSE;
	HANDLE hThread = NULL;
	PWSTR remoteFileName = NULL;
	__try
	{
		if (hProcess == NULL)
		{
			__leave;
		}

		int fileNameLength = lstrlenW(dllName) + 1;
		int bytesCount = fileNameLength * sizeof(wchar_t);

		remoteFileName = (PWSTR)VirtualAllocEx(hProcess, NULL, bytesCount, MEM_COMMIT, PAGE_READWRITE);
		if (remoteFileName == NULL)
		{
			__leave;
		}

		if (!WriteProcessMemory(hProcess, remoteFileName, (PVOID)dllName, bytesCount, NULL))
		{
			__leave;		
		}

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(_T("Kernel32")), "LoadLibraryW");

		if (pfnThreadRtn == NULL)
		{
			__leave;			
		}

		hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, remoteFileName, 0, NULL);		
		if (hThread == NULL)
		{
			__leave;
		}

		WaitForSingleObject(hThread, INFINITE);
		result = TRUE;
	}
	__finally
	{
		if (remoteFileName != NULL) VirtualFreeEx(hProcess, remoteFileName, 0, MEM_RELEASE);
		if (hThread != NULL) CloseHandle(hThread);
		//if (hProcess != NULL) CloseHandle(hProcess);
	}
	return result;
}


int _tmain(int argc, TCHAR *argv[])
{
	std::cout << "Name of program: ";
	if (argc > 1)
	{
		std::wcout << argv[1] << std::endl;
	}	

	//std::wstring arg(L"C:\\WINDOWS\\REGEDIT.EXE");
	//C:\\WINDOWS\\SYSTEM32\\NOTEPAD.EXE"
	//"C:\\WINDOWS\\REGEDIT.EXE"
	//std::wstring arg(L"C:/Program Files (x86)/Notepad++/notepad++.exe");
	std::wstring arg(L"E:\\Ждан Вова\\БГУИР\\3 курс\\5 семестр\\ОСиСП\\Лабораторные работы\\6-я лаба Hooks\\OSiSP_Hooks\\Debug\\Uninstaller.exe");
	const wchar_t * processName = arg.c_str();

	STARTUPINFO startUpInfo = { sizeof(startUpInfo) };
	SECURITY_ATTRIBUTES secAtrProcess, secAtrThread;
	PROCESS_INFORMATION procInfo;		

	secAtrProcess.nLength = sizeof(secAtrProcess);
	secAtrProcess.lpSecurityDescriptor = NULL;
	secAtrProcess.bInheritHandle = TRUE;
	secAtrThread.nLength = sizeof(secAtrThread);
	secAtrThread.lpSecurityDescriptor = NULL;
	secAtrThread.bInheritHandle = FALSE;	

	TCHAR commandLine[] = L"";	
		
	if (CreateProcess(processName, commandLine, &secAtrProcess, &secAtrThread, FALSE, CREATE_SUSPENDED, NULL, NULL, &startUpInfo, &procInfo))
	{
		std::cout << "program is running" << std::endl;;
	}
	else
	{
		std::cout << "error while starting program" << std::endl;
		int error = GetLastError();
	}			

	if (LoadDllToProcessUsingRemoteThread(procInfo.hProcess, L"E:/Ждан Вова/БГУИР/3 курс/5 семестр/ОСиСП/Лабораторные работы/6-я лаба Hooks/OSiSP_Hooks/Debug/OSiSP_Logging"))
	{
		std::cout << "loading dll finished" << std::endl;		
		ResumeThread(procInfo.hThread);
	}
	else
	{
		std::cout << "error while loading dll" << std::endl;
	}	

	if (procInfo.hThread != NULL)
	{
		CloseHandle(procInfo.hThread);
	}
	if (procInfo.hProcess != NULL)
	{
		CloseHandle(procInfo.hProcess);
	}

	system("pause");
	return 0;
}

