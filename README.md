# Login Monitor
Login Monitor is a Windows Service which allows to open notepad.exe app in user session.
Features:
- open exe in all active sessions on service start
- monitor new sessions login

### Build
VS 2019 and C++17

### Install
sc create <service name> binPath=<path to exe> //install service
sc config <service name> start=auto // automatic startup
sc delete <service name> // delete service
