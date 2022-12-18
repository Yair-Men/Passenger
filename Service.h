#ifndef service_h
#define service_h

#include <Windows.h>
#include <stdio.h>

class Service
{

	LPCSTR serviceName = { 0 };
	SC_HANDLE hSCM = nullptr;
	SC_HANDLE hService = nullptr;

public: 
	LPCSTR driverPath = { 0 };

public:
	Service(LPCSTR serviceName, LPCSTR driverPath);
	bool create_service();
	bool start_service();
	~Service();

};

#endif // !service_h
