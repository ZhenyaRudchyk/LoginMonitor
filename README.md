# Login Monitor
Login Monitor is a Windows Service which allows to open notepad.exe app in user session.

##### Features:
- open notepad.exe in all active sessions on service start
- monitor new sessions and open notepad.exe on log in

### Build
VS 2019 and C++17

### Install
sc create <service name> binPath=<path to exe> //install service
sc config <service name> start=auto // automatic startup
sc delete <service name> // delete service

#### P.S.
Tested only on Windows 10
