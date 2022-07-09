========================================================================
    ErrLib 
========================================================================

A library to assist in dealing with exceptions and errors in C/C++ Windows Applications

Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
License: BSD 3-clause
Requirements: Windows Vista (or newer), Visual Studio 2012 (or newer)

** Features **

- Simple way to get exception code, error message and stack trace in the handler block for SEH and C++ exceptions
- Executing user-defined callback function on unhandled SEH exception
- Converting WinAPI/COM errors into exceptions
- Configurable logging: can write diagnostic information into log file, stderr stream, Windows Event Log or other targets
- Multithreaded: all functionality can be used from different threads independently
- All string processing in Unicode (wide characters)

========================================================================

** Usage **

1. Copy header files (ErrLib.h and/or ErrLib_CPP.h) into one of your include file directories (or just into the project directory)
2. Copy the correct version of ErrLib.lib into one of your lib file directories (or just into the project directory)
3. Copy the correct version of ErrLib.dll into your project output directory (you must distribute it with your application on target machines)
4. Include header files in one or more of your modules and have fun using ErrLib functions (see documentation)!

Notes:

- You must use LIB and DLL file version matching your project target architecture (x86/x64) and configuration type (Debug/Release)
- ErrLib dynamically links to Visual C++ 2012 Standard Library (CRT) - debug or release depending on version. This means, either target machines must have Visual C++ 2012 Redistributable installed, or you must redistribute its DLLs along with your application. For any other deployment option, rebuid project from sources, changing configuration accordingly.
- You can use ErrLib via LoadLibrary/GetProcAddress if you really want, but don't call FreeLibrary. Unloading ErrLib before process termination is not supportted.

Simple usage example:

===============================================================================================================
#include <stdio.h>
#include "ErrLib.h"
#include "ErrLib_CPP.h"

// Function that throws exception when parameter value is invalid
float CalcRectangleArea(float width, float height){
    if(width<=0.0) throw ErrLib::Exception(L"Width must be positive");
    if(height<=0.0) throw ErrLib::Exception(L"Height must be positive");

    return width * height;
}

void func1(){
    float s = CalcRectangleArea(0.0, 2.0);
    wprintf(L"%f\n", s);
}

void func(){
    try
    {
        func1();
    }
    catch(ErrLib::Exception& ex)
    {
        // Catch exception and print diagnostic information
        wprintf(L"Exception: %s\n%s", ex.GetMsg().c_str(), ex.PrintStackTrace().c_str());
    }
}

int main()
{
    ErrLib_Initialize();

    func();

    getchar();
    return 0;
}

/* Example output:

Exception: Width must be positive
  in ErrLib_Demo.exe!CalcRectangleArea + 0x7c (c:\repos\errlib\errlib_demo\main.cpp; line: 10;)
  in ErrLib_Demo.exe!func1 + 0x3f (c:\repos\errlib\errlib_demo\main.cpp; line: 17;)
  in ErrLib_Demo.exe!func + 0x50 (c:\repos\errlib\errlib_demo\main.cpp; line: 25;)
  in ErrLib_Demo.exe!main + 0x28 (c:\repos\errlib\errlib_demo\main.cpp; line: 39;)
  in ErrLib_Demo.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 536;)
  in ErrLib_Demo.exe!mainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;)
  in KERNEL32.DLL!BaseThreadInitThunk (C:\WINDOWS\System32\KERNEL32.DLL; address: 0x7781fa29)
  in ntdll.dll!RtlGetAppContainerNamedObjectPath (C:\WINDOWS\SYSTEM32\ntdll.dll; address: 0x77967a9e)
  in ntdll.dll!RtlGetAppContainerNamedObjectPath (C:\WINDOWS\SYSTEM32\ntdll.dll; address: 0x77967a6e)
*/
===============================================================================================================

For more code examples, see Examples subdirectory.

Functions list is available in online API documentation: https://msdn-whiteknight.github.io/ErrLib/
