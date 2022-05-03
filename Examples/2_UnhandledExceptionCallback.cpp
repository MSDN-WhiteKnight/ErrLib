//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib unhandled exception callback example ***
//(run without debugging to see the results)

#include <stdio.h>
#include <tchar.h>
#include "ErrLib.h"

LONG WINAPI MyExceptionCallback ( struct _EXCEPTION_POINTERS * ex,LPCWSTR mes,LPCWSTR stack){
	     
    fwprintf(stderr,L"Unhandled exception: %s\n%s\n",mes,stack);  //display error information 		

	exit(1); //close application without invoking usual Windows Error Reporting crash message     
}

void func1(){
	int * p = NULL;	
    printf("%d",*p);
}

void func(){
	func1();
}

int _tmain(int argc, _TCHAR* argv[])
{
	ErrLib_Initialize();
	ErrLib_SetExceptionCallback(MyExceptionCallback);
				
	func();	
	
	return 0;
}

/* Output:
Unhandled exception: The instruction referenced memory at 0x00000000. The memory could not be read
  in ConsoleApplication1.exe!func1 + 0x2a (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 19;)
  in ConsoleApplication1.exe!func + 0x23 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 24;)
  in ConsoleApplication1.exe!wmain + 0x46 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 33;)
  in ConsoleApplication1.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 533;)
  in ConsoleApplication1.exe!wmainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;)
  in kernel32.dll!BaseThreadInitThunk (C:\Windows\syswow64\kernel32.dll; address: 0x74af343d)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9832)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9805)
*/

