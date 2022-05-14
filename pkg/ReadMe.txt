========================================================================
    ErrLib 
========================================================================

A library to assist in dealing with exceptions and errors in C/C++ Windows Applications

Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
License: BSD 3-clause
Requirements: Windows Vista (or newer), Visual Studio 2010 (or newer)

** Features **

- Simple way to get exception code, error message and stack trace in the handler block for SEH exceptions (as well as unhandled C++ exceptions which cause them)
- Executing user-defined callback function on unhandled SEH exception
- Helper macros for converting Win32/COM errors into exceptions
- Configurable logging: can write diagnostic information in log file, stderr stream, Windows Event Log or other targets
- Multithreaded: all functionality can be used from different threads independently)
- All string processing in Unicode (wide characters)

========================================================================

** Usage **

1. Copy ErrLib.h file into one of your include file directories (or just into the project directory)
2. Copy the correct version of ErrLib.lib into one of your lib file directories (or just into the project directory)
3. Copy the correct version of ErrLib.dll into your project output directory (you must distribute it with your application on target machines)
4. Include ErrLib.h in one of more of your modules and have fun using ErrLib functions (see below)!

Notes:

- You must use LIB and DLL file version matching your project target architecture (x86/x64) and configuration type (Debug/Release)
- ErrLib dynamically links to Visual C++ 2012 Standard Library (CRT) - debug or release depending on version. This means, either target machines must have Visual C++ 2012 Redistributable installed, or you must redistribute its DLLs along with your application. For any other deployment option, rebuid project from sources, changing configuration accordingly.
- You can use ErrLib via LoadLibrary/GetProcAddress if you really want, but don't call FreeLibrary. Unloading ErrLib before process termination is not supportted.

Simple usage example:

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

For more code examples, see "_Examples" subdirectory.

========================================================================

** Function list **

- ErrLib_ErrorMes function

Gets Win32 error message for specified function name and error code. The message depends on the OS locale.

    ERRLIB_API void __stdcall ErrLib_ErrorMes(LPTSTR lpszFunction,DWORD dw,WCHAR* buf);
	
lpszFunction - The name of the function which might have caused the error
dw - Error code returned by GetLastError()
buf - Pointer to character array to store the result string

- ErrLib_SetExceptionCallback function

Sets current exception callback - the user-defined function which will be called in case of unhandled SEH exception when there's no debugger attahed to the process. Specify NULL to call default callback.

    ERRLIB_API void __stdcall ErrLib_SetExceptionCallback(ERRLIB_EXCEPTION_CALLBACK pCallback);
	
pCallback - The pointer to callback function.

Remarks:

The callback function must be defined as follows:

    LONG WINAPI MyExceptionCallback ( struct _EXCEPTION_POINTERS * ex,LPCWSTR mes,LPCWSTR stack){
        ...   
    }

The parameter *ex* of callback function points to exception information structure. The *mes* and *stack* parameters point to the strings containing human-readable error message and stack trace respectively.

Return EXCEPTION_EXECUTE_HANDLER from callback function to execute default handler (in most cases in will crash application, invoking default Windows Error Reporting dialog window). Call exit function without returning anything, if you want to avoid this behaviour and terminate application normally.

The default callback writes exception information into configured logging targets and then crashes application.

- ErrLib_SetLogFilePath function

Sets current log file path. The default path is [MyDocuments]\[ExeFileName].log

    ERRLIB_API void __stdcall ErrLib_SetLogFilePath(LPCWSTR path);
	
Note: Only has any effect if ERRLIB_OUTPUT_LOGFILE configuration parameter is set to true.

- ErrLib_SetParameter function

Sets value for the specified configuration parameter.

    ERRLIB_API BOOL __stdcall ErrLib_SetParameter(UINT param, UINT_PTR value);

Parameters:
	
*param* - Paramater ID. Can be one of the following values:
 ERRLIB_OUTPUT_LOGFILE (1)		Specifies the logging functions should write information into the log file. Type: BOOL.
 ERRLIB_OUTPUT_STDERR (2)		Specifies the logging functions should write information into stderr stream (usually console). Type: BOOL.
 ERRLIB_OUTPUT_MBOX (3)			Specifies the logging functions should display information as message box. Type: BOOL.
 ERRLIB_OUTPUT_EVENT_LOG (4)	Specifies the logging functions should write information into Windows Event Log. Type: BOOL.
 
*value* - The new value for parameter being set. Use TRUE/FALSE constants cast to UINT_PTR for boolean parameters.
 
Return value - Non-zero value if operation succeeded, zero if parameter ID is not recognized.

- ErrLib_Initialize function

Initializes the library. Must be called before any other functionality is used.

    ERRLIB_API BOOL __stdcall ErrLib_Initialize();
	
Return value - Non-zero value if operation succeeded, zero if error occured.

Note: if you call any of the library functions without a prior call to ErrLib_Initialize, the behaviour is undefined.

- ErrLib_RegisterEventSource function

Creates registry data for Windows Event Log. Requires elevated priveleges. 

    ERRLIB_API BOOL __stdcall ErrLib_RegisterEventSource();
	
Return value - Non-zero value if operation succeeded, zero if error occured.

Note: This function fails if the program runs without elevated priveleges, but it sufficient to only call it once (for example, when application is installed).

- ErrLib_UnregisterEventSource function

Deletes Windows Event Log registry data. Requires elevated priveleges. 

    ERRLIB_API BOOL __stdcall ErrLib_UnregisterEventSource();
	
Note: the registry data is required not so much for writing into Event Log, but in order for the Event Viewer application to correctly display them. Therefore, do not call this function on every application exit; instead only do so when its events are no longer needed (i.e., when application is uninstalled).

- ErrLib_PrintStack function

Gets stack trace based on context record

    ERRLIB_API void __stdcall ErrLib_PrintStack( CONTEXT* ctx , WCHAR* dest, size_t cch);

Parameters:

*ctx* - The pointer to a CONTEXT structure, containing valid context record on input. You can obtain one via RtlCaptureContext or from exception data	

*dest* - The pointer to a caller-allocated character array that will be filled with stack trace text on output. Make sure to allocate a large enough block of memory, otherwise the text will be trimmed. The maximum supported stack trace length is indicated by ErrLib_StackLen constant

*cch* - The maximum amount of characters that can be put into an array pointed by *dest* parameter

- ErrLib_GetExceptionMessage function

Gets error message corresponding to the specified exception

    ERRLIB_API void __stdcall ErrLib_GetExceptionMessage(struct _EXCEPTION_POINTERS *ExceptionInfo, LPWSTR dest, size_t cch);
	
Parameters:

*ExceptionInfo* - a pointer to the exception information structure

*dest* - The pointer to a caller-allocated character array that will be filled with error message text on output. Make sure to allocate a large enough block of memory, otherwise the text will be trimmed. The maximum supported message length is indicated by ErrLib_MessageLen constant

*cch* - The maximum amount of characters that can be put into an array pointed by *dest* parameter

 - ErrLib_LogExceptionInfo function

Outputs exception information into configured log targets

    ERRLIB_API void __stdcall ErrLib_LogExceptionInfo(DWORD dwExcCode,LPCWSTR lpwsMessage,LPCWSTR lpwsStackTrace, BOOL visible);
	
Parameters:

*dwExcCode* - SEH Exception code. Use ErrLib_Except_GetCode to obtain it in exception handler block.
*lpwsMessage* - A pointer to the wide character string containing error message. Use ErrLib_Except_GetMessage to obtain it in exception handler block.
*lpwsStackTrace* - A pointer to the wide character string containing stack trace text. Use ErrLib_Except_GetStackTrace to obtain it in exception handler block.
*visible* - Pass TRUE if you want to use ERRLIB_OUTPUT_STDERR/ERRLIB_OUTPUT_MBOX logging targets (if they are enabled by configuration flags), FALSE otherwise

Remarks:

This function outputs information into one or more logging targets, configured using ErrLib_SetParameter function. If *visible* parameter is FALSE, it outputs only to log file and/or Windows Event Log (if respective flags are enabled). If *visible* parameter is TRUE, it additionally outputs information into stderr stream and/or message box (if respective flags are enabled). By default, the enabled logging targets are log file and stderr stream.  

When outputting information in message box, stack trace is not included.

ErrLib_LogMessage function

Outputs arbitrary string into configured log targets

    ERRLIB_API void __stdcall ErrLib_LogMessage(LPCWSTR lpwsMessage, BOOL visible, DWORD type, BOOL bIncludeStack );
	
Parameters:

*lpwsMessage* - A pointer to the wide character string containing message being written. 
*visible* - Pass TRUE if you want to use ERRLIB_OUTPUT_STDERR/ERRLIB_OUTPUT_MBOX logging targets (if they are enabled by configuration flags), FALSE otherwise
*type* - Can be: MSG_ERROR, MSG_WARNING or MSG_INFORMATION
*bIncludeStack* - Pass TRUE if you want to include stack trace into the logged information

This function outputs information into one or more logging targets, configured using ErrLib_SetParameter function. If *visible* parameter is FALSE, it outputs only to log file and/or Windows Event Log (if respective flags are enabled). If *visible* parameter is TRUE, it additionally outputs information into stderr stream and/or message box (if respective flags are enabled). By default, the enabled logging targets are log file and stderr stream.  

The *type* parameter affects message box or Windows Event Log entry appearance. For other targets, it has no effect.

When outputting information in message box, stack trace is not included, even if *bIncludeStack* parameter is true.

- ErrLib_Except_GetCode function

Gets exception code for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block

    ERRLIB_API DWORD __stdcall ErrLib_Except_GetCode();
	
Note: when used outside of the exception handler block, the behaviour is undefined.

- ErrLib_Except_GetMessage function

Gets error message for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block

    ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetMessage();
	
Note: when used outside of the exception handler block, the behaviour is undefined.

- ErrLib_Except_GetStackTrace function

Gets stack trace for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block

    ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetStackTrace();
	
Note: when used outside of the exception handler block, the behaviour is undefined.

- ERRLIB_THROW macro

Raises SEH exception with user-defined error message

    #define ERRLIB_THROW(mes) {...}
	
Note: the exception code is ERRLIB_APP_EXCEPTION  (0xC0400002)

- ERRLIB_THROW_IF_EQUAL macro

Raises Win32 exception if variable is equal to the passed value

    #define ERRLIB_THROW_IF_EQUAL(var,value,func) {...}
	
Parameters:

*var* - variable to be compared
*value* - a value for the variable to be compared against
*func* - the name of WINAPI function to be included in error message

Note: the exception code is ERRLIB_WIN32_EXCEPTION  (0xC0400000)

- ERRLIB_THROW_IF_FAILED macro

Raises COM exception if passed HRESULT indicates failure

    #define ERRLIB_THROW_IF_FAILED(var,func) {...}
	
Parameters:

*var* - HRESULT variable to be tested for failure
*func* - the name of WINAPI/COM function to be included in error message

Note: the exception code is ERRLIB_COM_EXCEPTION  (0xC0400001)

- ERRLIB_INVOKEAPI macro

Calls WINAPI function and raises Win32 exception if it fails. This only works for ones that return BOOL value indicating success/failure.

    #define ERRLIB_INVOKEAPI(func, ...) {...}

Parameters:

*func* - the name of WINAPI function to be called 
*...* - variable length list of arguments to be passed into the function

Note: If the function return anything else rather then BOOL value, use ERRLIB_THROW_IF_EQUAL instead. The exception code is ERRLIB_WIN32_EXCEPTION  (0xC0400000)

- ERRLIB_CATCH_ALL exception handler block

SEH custom _except block that executes handler for all exceptions

    #define ERRLIB_CATCH_ALL __except(ErrLib_CatchAll(GetExceptionInformation()))

- ERRLIB_CATCH exception handler block
	
SEH custom _except block that executes handler for exceptions with specified code

    #define ERRLIB_CATCH(code) __except(ErrLib_CatchCode(GetExceptionInformation(),code))


========================================================================
