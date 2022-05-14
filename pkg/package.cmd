@echo off
cd %~dp0
mkdir .\build\native\x86\
mkdir .\build\native\x64\
copy /Y /V ..\Release\ErrLib.dll /B build\native\x86\
copy /Y /V ..\Release\ErrLib.lib /B build\native\x86\
copy /Y /V ..\x64\Release\ErrLib.dll /B build\native\x64\
copy /Y /V ..\x64\Release\ErrLib.lib /B build\native\x64\
copy /Y /V ..\ErrLib\ErrLib.h .\build\
copy /Y /V ..\ErrLib\ReadMe.txt .\
"C:\Distr\Microsoft\nuget 3\nuget.exe" pack SmallSoft.ErrLib.nuspec
PAUSE
