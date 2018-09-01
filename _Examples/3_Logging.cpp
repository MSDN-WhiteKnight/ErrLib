//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib logging example ***

#include <stdio.h>
#include <tchar.h>
#include "ErrLib.h"

void func1(){
	throw std::exception();
}

void func(){
	func1();
}

int _tmain(int argc, _TCHAR* argv[])
{
	ErrLib_Initialize();
	
	//set custom log file path
	ErrLib_SetLogFilePath(L"d:\\error.log");
	
	//configure output to use message box instead of stderr (console)
	ErrLib_SetParameter(ERRLIB_OUTPUT_STDERR,FALSE);
	ErrLib_SetParameter(ERRLIB_OUTPUT_MBOX,TRUE);
	
	__try
	{
		func();	
	}
	ERRLIB_CATCH_ALL
	{
		ErrLib_LogExceptionInfo(ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace(),TRUE);
	}
	
	return 0;
}

/* 
*** Message box output: ***

Exception 0xe06d7363: C++ exception was not handled in user code

***************************

***  Log file output:   ***

2018.07.09 14:52 - Exception 0xe06d7363: C++ exception was not handled in user code
  in KERNELBASE.dll!RaiseException (C:\Windows\syswow64\KERNELBASE.dll; address: 0x7657c54f)
  in MSVCR110D.dll!CxxThrowException (C:\Windows\system32\MSVCR110D.dll; address: 0x5553b198)
  in ConsoleApplication1.exe!func1 + 0x44 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 11;)
  in ConsoleApplication1.exe!func + 0x23 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 15;)
  in ConsoleApplication1.exe!wmain + 0x9d (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 27;)
  in ConsoleApplication1.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 533;)
  in ConsoleApplication1.exe!wmainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;)
  in kernel32.dll!BaseThreadInitThunk (C:\Windows\syswow64\kernel32.dll; address: 0x74af343d)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9832)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9805)
  
***************************
*/

