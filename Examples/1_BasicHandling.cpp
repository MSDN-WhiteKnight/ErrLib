//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib basic exception handling example ***

#include <stdio.h>
#include <tchar.h>
#include "ErrLib.h"

void func1(){
	int a=0;
	int c;
	c = 1/a;
	printf("%d",c);
}

void func(){
	func1();
}

int _tmain(int argc, _TCHAR* argv[])
{
	ErrLib_Initialize();

	__try
	{		
		func();
	}
	ERRLIB_CATCH(EXCEPTION_INT_DIVIDE_BY_ZERO)
	{	
		wprintf(L"Exception 0x%x: %s\n%s\n",ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace());					
	}

	getchar();
	return 0;
}

/* Output:
Exception 0xc0000094: Integer division by zero
  in ConsoleApplication1.exe!func1 + 0x2b (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 8;)
  in ConsoleApplication1.exe!func + 0x23 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 14;)
  in ConsoleApplication1.exe!wmain + 0x63 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 23;)
  in ConsoleApplication1.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 533;)
  in ConsoleApplication1.exe!wmainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;)
  in kernel32.dll!BaseThreadInitThunk (C:\Windows\syswow64\kernel32.dll; address: 0x74af343d)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9832)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9805)
*/