#ifndef driver_h
#define drvier_h

#include <Windows.h>
#include <winternl.h>
#include <stdio.h>
#pragma comment(lib, "ntdll.lib")

// DEFS
#define IOCTL_OPEN_PROTECTED_PROCESS_HANDLE 0x8335003c 
#define IOCTL_DUPLICATE_TOKEN 0x8335000c
#define IOCTL_CLOSE_HANDLE 0x83350004

#define SystemHandleInformationSize 1024 * 1024 * 2
#define SystemHandleInformation 16
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004


//STRUCTS
typedef struct my__PUBLIC_OBJECT_TYPE_INFORMATION {
    UNICODE_STRING TypeName;
    ULONGLONG Reserved[22];    // reserved for internal use
} myPUBLIC_OBJECT_TYPE_INFORMATION, * myPPUBLIC_OBJECT_TYPE_INFORMATION;

typedef struct procexp_close_handle {
    ULONGLONG pPid = 0x0;
    PVOID ObjectType;
    ULONGLONG nothing2 = 0x0;
    ULONGLONG handle;
} Procexp_close, * pProcexp_close;

using fNtQuerySystemInformation = NTSTATUS(WINAPI*)(
    ULONGLONG SystemInformationClass,
    PVOID SystemInformation,
    ULONGLONG SystemInformationLength,
    PULONGLONG ReturnLength
    );

/*Define NtQueryProcessInformation*/

using fNtQueryProcessInformation = NTSTATUS(WINAPI*)(
    HANDLE           ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID            ProcessInformation,
    ULONG            ProcessInformationLength,
    PULONG           ReturnLength
    );

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;
    UCHAR HandleAttributes;
    USHORT Handle;
    PVOID Object;
    ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
    long NumberOfHandles;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;




// FUNCTIONS

HANDLE open_handle(ULONGLONG PID, HANDLE hDriver);
BOOL kill(HANDLE hDriver, HANDLE hProc, ULONGLONG PID);

#endif // !drvier_h