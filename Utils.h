#ifndef utils_h
#define utils_h

#pragma warning(disable:4996)
#include <Windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <string>

//typedef NTSTATUS(WINAPI* fNtSuspendProcess) ( HANDLE hProc );

using fNtSuspendProcess = NTSTATUS(WINAPI*)(
    HANDLE hProc
    );

BOOL getProcessNameById(ULONG pid, WCHAR *procName);
DWORD getProcessIdByName(WCHAR* ProcName);
char* getCmdParam(char** begin, char** end, const std::string& option);
bool cmdParamExists(char** begin, char** end, const std::string& option);
void print_help();
BOOL SetDebugPrivilege();

#endif // !utils_h