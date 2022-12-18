#include "Service.h"


Service::Service(LPCSTR serviceName, LPCSTR driverPath)
{
	hSCM = OpenSCManagerA(".", NULL, SC_MANAGER_CREATE_SERVICE);
	if (hSCM == nullptr) {
		printf("[-] Failed to get handle to SCM. (Error Code: %i)\n", GetLastError());
		exit(0);
	}
		
	this->hSCM = hSCM;
	this->serviceName = serviceName == 0 ? "PROCEXP152" : serviceName;
	this->driverPath = driverPath == 0 ? "C:\\temp\\PROCEXP152.sys" : driverPath;
	this->hService = nullptr;

	this->hService = OpenServiceA(hSCM, this->serviceName, SERVICE_START);
}

bool Service::create_service() {
	
	if (this->hService != nullptr) {
		printf("[!] Service allready exists\n");
		return this->start_service();
	}

	this->hService = CreateServiceA(this->hSCM, this->serviceName, this->serviceName, SC_MANAGER_CREATE_SERVICE | SERVICE_START, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, this->driverPath, NULL, NULL, NULL, NULL, NULL);

	DWORD errCode = GetLastError();

	if (this->hService == nullptr) {
		printf("[-] Failed To Create Service. (Error Code: %i)\n", GetLastError());
	}
	else if (errCode == 1073) { // 1073 == ERROR_SERVICE_EXISTS
		printf("[!] Service allready exists\n");
		return this->start_service();
	}
	else if (errCode == 0) {
		printf("[+] Service Created Successfully\n");
		return this->start_service();
	}
	else if (errCode != 0) {
		printf("[!] Hit last else if in create service. (Error Code: %i)\n", GetLastError());
	}

	return FALSE;
}

bool Service::start_service() {
	
	BOOL OK = StartServiceA(this->hService, 0, NULL);

	if (!OK)
	{
		DWORD errCode = GetLastError();

		// 6 = ERROR_INVALID_HANDLE, 1056 = ERROR_SERVICE_ALREADY_RUNNING
		if (errCode == 6 || errCode == 1056) { 
			printf("[!] Service allready running\n");
			return TRUE;
		}
		else {
			printf("[-] Failed to start Service. (Error Code: %i)\n", errCode);
			return FALSE;
		}
	}

	printf("[+] Service started successfully\n");
	return TRUE;
}

Service::~Service()
{
	if (this->hService != nullptr) CloseServiceHandle(this->hService);
	if (this->hSCM != nullptr) CloseServiceHandle(this->hSCM);
}