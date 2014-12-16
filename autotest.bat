@echo off

set targetdir=d:\bitcoin
set curdir=%cd%
set sever=%cd%\soypayd.exe
set autotestexe=%cd%\src\test\soypay_test.exe
set autosevertestexe=%cd%\src\ptest\soypay_ptest.exe
set movefiles= %curdir%\src\test\data\*.bin


call :ClrEnvironment
exit /b 0


:RunTestSuite

start  %autotestexe%  %1
GOTO :EOF

:CloseServer
taskkill /f /im soypayd.exe
GOTO :EOF

:StartServer
echo Clr environment
start  %sever% -datadir=%targetdir%
ping 127.0.0.1 -n 3 >nul 

GOTO :EOF


:ClrEnvironment
echo Clr environment
rd /s/q %targetdir%\regtest & md %targetdir%\regtest
cp  %curdir%\src\test\data\soypay.conf %targetdir%\soypay.conf
copy %movefiles% %targetdir%\data\

echo Clr environment OK
GOTO :EOF

echo  %cd%

pause

