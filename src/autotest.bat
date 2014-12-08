@echo off

set targetdir=d:\bitcoin
set curdir=%cd%
set sever=%cd%\soypayd.exe
set autotestexe=%cd%\test\soypay_test.exe
set autosevertestexe=%cd%\ptest\soypay_ptest.exe



call :ClrEnvironment
call :StartServer
%autosevertestexe% --run_test=test_rollback/db_fun
call :CloseServer

call :ClrEnvironment
call :StartServer
%autosevertestexe% --run_test=test_app/test_dark
call :CloseServer

call :ClrEnvironment
call :StartServer
%autosevertestexe% --run_test=test_app/test_anony
call :CloseServer

call :ClrEnvironment
call :StartServer
%autosevertestexe% --run_test=test_script/script_account
call :CloseServer

call :ClrEnvironment
call :StartServer
%autosevertestexe% --run_test=VM_fun/test_fun
call :CloseServer
pause
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
cp  %curdir%\test\data\wallet.dat %targetdir%\wallet.dat
cp  %curdir%\test\data\wallet.dat %targetdir%\regtest\wallet.dat
cp  %curdir%\ptest\data\anony.bin %targetdir%\data\anony.bin
cp  %curdir%\ptest\data\darksecure.bin %targetdir%\data\darksecure.bin
cp  %curdir%\ptest\data\scripttest.bin %targetdir%\data\scripttest.bin
cp  %curdir%\ptest\data\sdk.bin %targetdir%\data\sdk.bin
cp  %curdir%\ptest\data\testrollback.bin %targetdir%\data\testrollback.bin
cp  %curdir%\ptest\data\testscriptid.bin %targetdir%\data\testscriptid.bin
echo Clr environment OK
GOTO :EOF



echo  %cd%

pause

