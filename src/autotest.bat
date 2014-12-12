@echo off

set targetdir=d:\bitcoin
set curdir=%cd%
set sever=%cd%\soypayd.exe
set autotestexe=%cd%\test\soypay_test.exe
set autosevertestexe=%cd%\ptest\soypay_ptest.exe



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
cp  %curdir%\test\data\soypay.conf %targetdir%\soypay.conf
cp  %curdir%\test\data\anony.bin %targetdir%\data\anony.bin
cp  %curdir%\test\data\darksecure.bin %targetdir%\data\darksecure.bin
cp  %curdir%\test\data\scripttest.bin %targetdir%\data\scripttest.bin
cp  %curdir%\test\data\soypay_test.bin %targetdir%\data\soypay_test.bin
echo Clr environment OK
GOTO :EOF



echo  %cd%

pause

