## About

A library to assist in dealing with exceptions and errors in C/C++ Windows Applications

**Author:** MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

**License:** BSD 3-clause

**Requirements:** Windows Vista or newer, Visual Studio 2012+ (VS 2015+ or NuGet 3.3+ for NuGet package)

## Features

- Simple way to get exception code, error message and stack trace in the handler block for SEH exceptions (as well as unhandled C++ exceptions which cause them)
- Executing user-defined callback function on unhandled SEH exception
- Helper macros for converting Win32/COM errors into exceptions
- Configurable logging: can write diagnostic information into log file, stderr stream, Windows Event Log or other targets
- Multithreaded: all functionality can be used from different threads independently
- All string processing in Unicode (wide characters)

## Usage

Include ErrLib.h in one of more of your modules and have fun using ErrLib functions (see below)!

Notes:

- ErrLib dynamically links to Visual C++ 2012 Standard Library (CRT) - debug or release depending on version. This means, either target machines must have Visual C++ 2012 Redistributable installed, or you must redistribute its DLLs along with your application. For any other deployment option, rebuid project from sources, changing configuration accordingly.
- You can use ErrLib via LoadLibrary/GetProcAddress if you really want, but don't call FreeLibrary. Unloading ErrLib before process termination is not supported.
- Enable full debug information in Visual Studio 2017 and above for complete stack traces (/DEBUG:FULL in linker options).

## Example

```
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
```

For more information, see [Documentation](https://github.com/MSDN-WhiteKnight/ErrLib/blob/master/README.md) and [Examples](https://github.com/MSDN-WhiteKnight/ErrLib/tree/master/Examples).
