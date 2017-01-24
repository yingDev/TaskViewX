# TaskViewX
Windows 10 Task View eXtension

# W.I.P

![](https://github.com/yingDev/TaskViewX/blob/master/.readme/screen.jpg?raw=true)

# Contribute
## Prerequisites
- QT 5.7
- Visual Studio 2015 + Qt Vs Tools
- Add an environment variable `QTDIR` pointing at ... eg. `C:\Qt\Qt5.7.0\5.7\msvc2015_64`
- Add to Path: `%QTDIR%\bin`
- `Start Menu > VS2015 Cmd Prompt >`, `cd TaskViewX`, `makeTestCert.bat`

## Build & Debug
To debug TaskViewX, you need to do some hack...
- Run Visual Studio `As Administrator`
- `Build > Build Solution`
- Go to `C:\Program Files\_TaskViewX_Tem\x64\Debug` and run `TaskViewX.exe`
- `VS > Debug > Attach to Process`

# Features Currently Available
- Double tap `Shift` to show Task View
- Press corresponding keys to "Switch to" windows
