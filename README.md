# ErrLib

[![Nuget](https://img.shields.io/nuget/v/SmallSoft.ErrLib)](https://www.nuget.org/packages/SmallSoft.ErrLib/)

[Documentation](https://msdn-whiteknight.github.io/ErrLib/) | [Examples](/Examples/)

A library to assist in dealing with exceptions and errors in C/C++ Windows Applications

**Author:** MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

**License:** BSD 3-clause

**Requirements:** Windows Vista (or newer), Visual Studio 2010 (or newer)

## Features

- Simple way to get exception code, error message and stack trace in the handler block for SEH exceptions (as well as unhandled C++ exceptions which cause them)
- Executing user-defined callback function on unhandled SEH exception
- Helper macros for converting Win32/COM errors into exceptions
- Configurable logging: can write diagnostic information into log file, stderr stream, Windows Event Log or other targets
- Multithreaded: all functionality can be used from different threads independently
- All string processing in Unicode (wide characters)

-----------------------------------------------------------------------------

## Usage

1. Copy ErrLib.h file into one of your include file directories (or just into the project directory)
2. Copy the correct version of ErrLib.lib into one of your lib file directories (or just into the project directory)
3. Copy the correct version of ErrLib.dll into your project output directory (you must distribute it with your application on target machines)
4. Include ErrLib.h in one of more of your modules and have fun using ErrLib functions (see documentation)!

**Notes:**

- You must use LIB and DLL file version matching your project target architecture (x86/x64) and configuration type (Debug/Release)
- ErrLib dynamically links to Visual C++ 2012 Standard Library (CRT) - debug or release depending on version. This means, either target machines must have Visual C++ 2012 Redistributable installed, or you must redistribute its DLLs along with your application. For any other deployment option, rebuid project from sources, changing configuration accordingly.
- You can use ErrLib via LoadLibrary/GetProcAddress if you really want, but don't call FreeLibrary. Unloading ErrLib before process termination is not supported.

Simple usage example:

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

For more code examples, see [Examples](/Examples/) subdirectory. For a list of functions, see [API Documentation](https://msdn-whiteknight.github.io/ErrLib/_err_lib_8h.html#func-members).
