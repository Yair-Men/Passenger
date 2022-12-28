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

	this->hService = OpenServiceA(hSCM, this->serviceName, SERVICE_START|SERVICE_CHANGE_CONFIG);
}

bool Service::create_service() {
	
	if (this->hService != nullptr) {
		printf("[!] Service allready exists\n");
		return this->start_service();
	}

	this->hService = CreateServiceA(this->hSCM, this->serviceName, this->serviceName, SC_MANAGER_CREATE_SERVICE|SERVICE_START|SERVICE_CHANGE_CONFIG, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, this->driverPath, NULL, NULL, NULL, NULL, NULL);

	DWORD errCode = GetLastError();

	if (this->hService == nullptr) {
		printf("[-] Failed To Create Service. (Error Code: %lu)\n", GetLastError());
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
		printf("[!] Hit last else if in create service. (Error Code: %lu)\n", GetLastError());
	}

	return FALSE;
}

bool Service::start_service() {
	
	BOOL OK = StartServiceA(this->hService, 0, NULL);
	
	if (!OK)
	{
		DWORD errCode = GetLastError();

		// 183 == ERROR_ALLREADY_EXISTS, 1056 = ERROR_SERVICE_ALREADY_RUNNING
		if (errCode == 183 || errCode == 1056) {
			printf("[!] Service allready running\n");
			return TRUE;
		} 
		else if(errCode == 1058) { // 1058 == ERROR_SERVICE_DISABLED
			printf("[!] Service is in disabled state\n");
			if(this->enable_service()) return TRUE;
		}
		else {
			printf("[-] Failed to start Service. (Error Code: %lu)\n", errCode);
			return FALSE;
		}
	}

	printf("[+] Service started successfully\n");
	return TRUE;
}

// We only get here when the service marked for deletion, Windows disables the service
bool Service::enable_service() {
	BOOL OK = ChangeServiceConfigA(this->hService, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
	// When errCode == 1072 (ERROR_SERVICE_MARKED_FOR_DELETE) it will be start anyway, but we will need this hack everytime unless a new service will be created
	if (!OK && GetLastError() != 1072) {
		printf("[-] Failed to change service config. (Error Code: %lu)", GetLastError());
		printf("[!] Try to install the service again with different name or use sc.exe to start the service\n");
		return FALSE;
	}
	printf("[+] Service enabled once again\n");
	return TRUE;
}

Service::~Service()
{
	if (this->hService != nullptr) CloseServiceHandle(this->hService);
	if (this->hSCM != nullptr) CloseServiceHandle(this->hSCM);
}