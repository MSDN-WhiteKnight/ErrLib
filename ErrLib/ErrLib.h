//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

/** \file ErrLib.h
 * ErrLib public API
 */

#ifndef ErrLib_H_INCLUDED
#define ErrLib_H_INCLUDED
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <windows.h>
#include <strsafe.h>
#include <process.h>
#include <DbgHelp.h>
#include <Shlobj.h>

#ifdef __cplusplus
#include <exception>
#include <comdef.h>
#define ERRLIB_REFERENCE
#define ERRLIB_INLINE inline
#else
#define ERRLIB_REFERENCE &
#define ERRLIB_INLINE __inline
#endif

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Dbghelp.lib")

#define ErrLib_MaxNameLen 300

/**
 * Specifies the maximum message length supported by the ErrLib_GetExceptionMessage function
 */
#define ErrLib_MessageLen 1024

/**
 * Specifies the maximum stack trace length supported by the ErrLib_PrintStack function
 */
#define ErrLib_StackLen 10000

#ifdef ERRLIB_EXPORTS
#define ERRLIB_API  __declspec(dllexport) 
#else
#define ERRLIB_API  __declspec(dllimport)
#pragma comment(lib, "ErrLib.lib")
#endif

// *** The following are the message definitions. ***

/**
 * Message type: Error (used with ErrLib_LogMessage)
 */
#define MSG_ERROR                        ((DWORD)0xC0020100L)

/**
 * Message type: Warning (used with ErrLib_LogMessage)
 */
#define MSG_WARNING                      ((DWORD)0x80020101L)

/**
 * Message type: Information (used with ErrLib_LogMessage)
 */
#define MSG_INFORMATION                  ((DWORD)0x40020102L)

// *** Configuration flags *** 

/**
 * Specifies that logging functions should write information into the log file
 * @note This configuration parameter is used with ErrLib_SetParameter function (type: BOOL).
 */
#define ERRLIB_OUTPUT_LOGFILE 1

/**
 * Specifies that logging functions should write information into the stderr stream
 * @note This configuration parameter is used with ErrLib_SetParameter function (type: BOOL).
 */
#define ERRLIB_OUTPUT_STDERR 2

/**
 * Specifies that logging functions should display information as a message box
 * @note This configuration parameter is used with ErrLib_SetParameter function (type: BOOL).
 */
#define ERRLIB_OUTPUT_MBOX 3

/**
 * Specifies that logging functions should write information into Windows Event Log
 * @note This configuration parameter is used with ErrLib_SetParameter function (type: BOOL).
 */
#define ERRLIB_OUTPUT_EVENT_LOG 4

//Specifies that logging functions should invoke custom logging callback 
#define ERRLIB_OUTPUT_CUSTOM 5

#define ERRLIB_PARAM_VISUALCPPVERSION 100
#define ERRLIB_PARAM_ISDEBUGBUILD 101

/**
 * Stack frame property: Symbol name
 */
#define ERRLIB_SYMBOL_NAME   1

/**
 * Stack frame property: Module file path
 */
#define ERRLIB_SYMBOL_MODULE 2

/**
 * Stack frame property: Symbol source file path
 */
#define ERRLIB_SYMBOL_SOURCE 3

// *** Typedefs *** 

//Function pointer type used as unhandled exception callback
typedef LONG  (WINAPI * ERRLIB_EXCEPTION_CALLBACK) ( struct _EXCEPTION_POINTERS *,LPCWSTR,LPCWSTR);

//Function pointer type used for custom logging targets
typedef void (WINAPI * ERRLIB_LOGGING_CALLBACK) (LPCWSTR, void*);

/**
 * Represents a stack frame, an object that contains information about an individual call in stack trace
 * @note Do not access the fields of this structure directly, they are considered private implementation details. 
 * @note Use ErrLib_ST_... functions instead to get stack frame properties.
 */
typedef struct structERRLIB_STACK_FRAME{
    uint64_t addr;
    uint64_t displacement;
    WCHAR symbol[MAX_SYM_NAME];
    WCHAR module[MAX_PATH];
    WCHAR src_file[MAX_PATH];
    DWORD src_line;
} ERRLIB_STACK_FRAME;

/**
 * Represents a stack trace, a chain of function calls at the particular point of thread's execution
 * @note Do not access the fields of this structure directly, they are considered private implementation details. 
 * @note Use ErrLib_ST_... functions instead to get stack trace properties.
 */
typedef struct structERRLIB_STACK_TRACE{
    ERRLIB_STACK_FRAME *data;
    int capacity;
    int count;
    BOOL isOnHeap;
} ERRLIB_STACK_TRACE;

// *** Custom exception codes for SEH *** 

//Win32 Exception
#define ERRLIB_WIN32_EXCEPTION  0xC0400000 

//Com Exception
#define ERRLIB_COM_EXCEPTION  0xC0400001

//Application Exception
#define ERRLIB_APP_EXCEPTION  0xC0400002 

// *** exported function declarations *** 

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets Win32 error message for the specified function name and error code. The message depends on the OS locale.
 * 
 * @param lpszFunction The name of the function which might have caused the error
 * @param dw Error code returned by GetLastError()
 * @param buf Pointer to character array to store the result string
 */
ERRLIB_API void __stdcall ErrLib_ErrorMes(LPTSTR lpszFunction,DWORD dw,WCHAR* buf);

ERRLIB_API DWORD __stdcall ErrLib_GetWinapiErrorMessage(DWORD dwCode, BOOL localized, WCHAR* pOutput, int cch);

//Gets filename from full path
ERRLIB_API WCHAR* __stdcall ErrLib_FileNameFromPathW(WCHAR* path);

//Sets current logging callback
ERRLIB_API void __stdcall ErrLib_SetLoggingCallback(ERRLIB_LOGGING_CALLBACK pCallback);

/**
 * Sets current exception callback - the user-defined function which will be called in case of unhandled SEH exception when there's no debugger attahed to the process. 
 * Specify NULL to call default callback.
 * 
 * @param pCallback The pointer to callback function
 * 
 * The callback function must be defined as follows: `LONG WINAPI MyExceptionCallback (struct _EXCEPTION_POINTERS* ex, LPCWSTR mes, LPCWSTR stack) {...}`
 *
 * The **ex** parameter of the callback function points to the exception information structure. 
 * The **mes** and **stack** parameters point to strings containing human-readable error message and stack trace respectively.
 *
 * Return EXCEPTION_EXECUTE_HANDLER from callback function to execute default handler (in most cases it will crash application, invoking default Windows Error Reporting 
 * dialog window). Call `exit` function without returning anything, if you want to avoid this behaviour and terminate application normally.
 *
 * The default callback writes exception information into configured logging targets and then crashes application.
 */
ERRLIB_API void __stdcall ErrLib_SetExceptionCallback(ERRLIB_EXCEPTION_CALLBACK pCallback);

/**
 * Sets current log file path. The default is `[MyDocuments]\[ExeFileName].log`
 * 
 * @note Only has any effect if ERRLIB_OUTPUT_LOGFILE configuration parameter is set to TRUE.
 */
ERRLIB_API void __stdcall ErrLib_SetLogFilePath(LPCWSTR path);

/**
 * Sets a value for the specified configuration parameter
 * 
 * @param param Paramater ID
 * @param value The new value for the parameter being set. Use TRUE/FALSE constants cast to UINT_PTR for boolean parameters.
 * @returns Non-zero value if operation succeeded, zero if parameter ID is not recognized or error occured.
 *
 * The paramater ID can be one of the following values:
 * - ERRLIB_OUTPUT_LOGFILE (1). Specifies that logging functions should write information into the log file. Type: BOOL.
 * - ERRLIB_OUTPUT_STDERR (2). Specifies that logging functions should write information into stderr stream (usually console). Type: BOOL.
 * - ERRLIB_OUTPUT_MBOX (3). Specifies that logging functions should display information as message box. Type: BOOL.
 * - ERRLIB_OUTPUT_EVENT_LOG (4). Specifies that logging functions should write information into Windows Event Log. Type: BOOL.
 */
ERRLIB_API BOOL __stdcall ErrLib_SetParameter(UINT param, UINT_PTR value);

//Initializes the library.
ERRLIB_API BOOL __stdcall ErrLib_InitializeInternal();

/**
 * Initializes the library. Must be called before any other functionality is used.
 * 
 * @returns Non-zero value if operation succeeded, zero if error occured
 * @note If you call any of the library functions without a prior call to ErrLib_Initialize, the behaviour is undefined.
 */
BOOL ERRLIB_INLINE ErrLib_Initialize(){
    BOOL ret = ErrLib_InitializeInternal();
#ifdef _MSC_VER
    ErrLib_SetParameter(ERRLIB_PARAM_VISUALCPPVERSION, (UINT_PTR)_MSC_VER);
#endif

#ifdef _DEBUG
    ErrLib_SetParameter(ERRLIB_PARAM_ISDEBUGBUILD, (UINT_PTR)TRUE);
#endif

    return ret;
}

ERRLIB_API BOOL __stdcall ErrLib_InitTLS();
ERRLIB_API BOOL __stdcall ErrLib_InitThread();
ERRLIB_API void __stdcall ErrLib_FreeThread();

/**
 * Creates registry data for Windows Event Log. Requires elevated priveleges.
 * 
 * @returns Non-zero value if operation succeeded, zero if error occured
 * @note This function fails if the program runs without elevated priveleges, but it sufficient to only call it once (for example, when application is installed).
 */
ERRLIB_API BOOL __stdcall ErrLib_RegisterEventSource();

/**
 * Deletes Windows Event Log registry data. Requires elevated priveleges.
 * 
 * @returns Non-zero value if operation succeeded, zero if error occured
 * @note The registry data is required not so much for writing into Event Log, but in order for the Event Viewer application to correctly display them. 
 * Therefore, do not call this function on every application exit; instead only do so when its events are no longer needed (i.e., when application is uninstalled).
 */
ERRLIB_API BOOL __stdcall ErrLib_UnregisterEventSource();

/**
 * Gets the stack trace information for the specified context record
 * 
 * @param ctx The pointer to a CONTEXT structure, containing valid context record on input
 * @returns The structure that contains stack trace information
 * @note You can obtain a context record via [RtlCaptureContext](https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlcapturecontext) function 
 * or from the exception data.
 * @note When you no longer need the stack trace information, free resources associated with it by calling ErrLib_FreeStackTrace.
 */
ERRLIB_API ERRLIB_STACK_TRACE __stdcall ErrLib_GetStackTrace(CONTEXT* ctx);

/**
 * Gets the number of stack frames in the specified stack trace
 * 
 * @param pStack The pointer to a ERRLIB_STACK_TRACE structure
 * @returns The integer number of stack frames
 */
ERRLIB_API int __stdcall ErrLib_ST_GetFramesCount(const ERRLIB_STACK_TRACE* pStack);

/**
 * Gets the frame with the specified number from a stack trace
 * 
 * @param pStack The pointer to a ERRLIB_STACK_TRACE structure
 * @param n The frame number to get
 * @param pOutput The pointer to a ERRLIB_STACK_FRAME structure that will contain the output data
 * @returns TRUE on success or FALSE when error occured
 */
ERRLIB_API BOOL  __stdcall ErrLib_ST_GetFrame(const ERRLIB_STACK_TRACE* pStack, int n, ERRLIB_STACK_FRAME* pOutput);

/**
 * Gets the symbol address from the specified stack frame
 * 
 * @param pFrame The pointer to a ERRLIB_STACK_FRAME structure
 * @returns The 64-bit unsigned integer the represents the symbol address
 */
ERRLIB_API uint64_t __stdcall ErrLib_ST_GetAddress(const ERRLIB_STACK_FRAME* pFrame);

/**
 * Gets the stack frame displacement from the symbol address (the difference between the instruction pointer value and 
 * the starting address of the function)
 * 
 * @param pFrame The pointer to a ERRLIB_STACK_FRAME structure
 * @returns The 64-bit unsigned integer the represents the stack frame displacement
 */
ERRLIB_API uint64_t __stdcall ErrLib_ST_GetDisplacement(const ERRLIB_STACK_FRAME* pFrame);

ERRLIB_API int __stdcall ErrLib_ST_GetStringProperty(const ERRLIB_STACK_FRAME* pFrame, int propId, WCHAR* pOutput, int cch);

/**
 * Gets the source line number from the specified stack frame
 * 
 * @param pFrame The pointer to a ERRLIB_STACK_FRAME structure
 * @returns The unsigned integer the represents the source line number
 */
ERRLIB_API DWORD __stdcall ErrLib_ST_GetSymLine(const ERRLIB_STACK_FRAME* pFrame);

/**
 * Releases resources associated with the specified stack trace structure
 * 
 * @param pStack The pointer to a ERRLIB_STACK_TRACE structure
 * @note Do not pass the stack trace into any other functions after it has been freed by this function.
 */
ERRLIB_API void __stdcall ErrLib_FreeStackTrace(ERRLIB_STACK_TRACE* pStack);

/**
 * Prints stack trace for the specified context record
 * 
 * @param ctx The pointer to a CONTEXT structure, containing valid context record on input. 
 * @param dest The pointer to the caller-allocated character array that will be filled with stack trace text on output. 
 * Make sure to allocate a large enough block of memory, otherwise the text will be trimmed. The maximum supported stack trace length is indicated by ErrLib_StackLen constant.
 * @param cch The maximum amount of characters that can be put into the array pointed by **dest** parameter
 *
 * @note You can obtain a context record via [RtlCaptureContext](https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlcapturecontext) function 
 * or from the exception data.
 */
ERRLIB_API void __stdcall ErrLib_PrintStack(CONTEXT* ctx, WCHAR* dest, size_t cch);

/**
 * Prints error message for specified exception
 * 
 * @param ExceptionInfo A pointer to the exception information structure
 * @param dest The pointer to the caller-allocated character array that will be filled with error message text on output. 
 * Make sure to allocate a large enough block of memory, otherwise the text will be trimmed. The maximum supported message length is indicated by ErrLib_MessageLen constant.
 * @param cch The maximum amount of characters that can be put into the array pointed by **dest** parameter
 */
ERRLIB_API void __stdcall ErrLib_GetExceptionMessage(struct _EXCEPTION_POINTERS* ExceptionInfo, LPWSTR dest, size_t cch);

/**
 * Outputs the exception information into configured log targets
 * 
 * @param dwExcCode SEH Exception code. Use ErrLib_Except_GetCode to obtain it in exception handler block.
 * @param lpwsMessage A pointer to the wide character string containing error message. Use ErrLib_Except_GetMessage to obtain it in exception handler block.
 * @param lpwsStackTrace A pointer to the wide character string containing stack trace text. Use ErrLib_Except_GetStackTrace to obtain it in exception handler block.
 * @param visible Pass TRUE if you want to use ERRLIB_OUTPUT_STDERR/ERRLIB_OUTPUT_MBOX logging targets (if they are enabled by configuration flags), FALSE otherwise
 *
 * This function outputs information into one or more logging targets, configured using ErrLib_SetParameter function.
 * It only outputs information into the stderr stream and message box if the **visible** parameter is TRUE (and if respective flags are enabled).
 * By default, the enabled logging targets are log file and stderr stream. 
 * @note When outputting information in message box, stack trace is not included. 
 */
ERRLIB_API void __stdcall ErrLib_LogExceptionInfo(DWORD dwExcCode,LPCWSTR lpwsMessage,LPCWSTR lpwsStackTrace, BOOL visible);

/**
 * Outputs arbitrary string into configured log targets
 * 
 * @param lpwsMessage A pointer to the wide character string containing message being written
 * @param visible Pass TRUE if you want to use ERRLIB_OUTPUT_STDERR/ERRLIB_OUTPUT_MBOX logging targets (if they are enabled by configuration flags), FALSE otherwise
 * @param type Can be: MSG_ERROR, MSG_WARNING or MSG_INFORMATION
 * @param bIncludeStack Pass TRUE if you want to include stack trace into the logged information
 *
 * This function outputs information into one or more logging targets, configured using ErrLib_SetParameter function.
 * It only outputs information into the stderr stream and message box if the **visible** parameter is TRUE (and if respective flags are enabled).
 * By default, the enabled logging targets are log file and stderr stream. 
 * @note The **type** parameter affects message box or Windows Event Log entry appearance. For other targets, it has no effect. 
 * @note When outputting information in message box, stack trace is not included, even if **bIncludeStack** parameter is TRUE.
 */
ERRLIB_API void __stdcall ErrLib_LogMessage(LPCWSTR lpwsMessage, BOOL visible, DWORD type, BOOL bIncludeStack );

/**
 * Gets the exception code for the current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
 * 
 * @note When used outside of the exception handler block, the behaviour is undefined
 */
ERRLIB_API DWORD __stdcall ErrLib_Except_GetCode();

/**
 * Gets the error message for the current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
 * 
 * @note When used outside of the exception handler block, the behaviour is undefined
 */
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetMessage();

/**
 * Gets the stack trace text for the current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
 * 
 * @note When used outside of the exception handler block, the behaviour is undefined.
 */
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetStackTrace();

/**
 * Gets the stack trace information for the current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
 * 
 * @returns The structure that contains stack trace information
 * @note When you no longer need the stack trace information, free resources associated with it by calling ErrLib_FreeStackTrace.
 * @note When used outside of the exception handler block, the behaviour is undefined.
 */
ERRLIB_API ERRLIB_STACK_TRACE __stdcall ErrLib_Except_GetStackTraceData();

ERRLIB_API LONG __stdcall ErrLib_CatchCode( struct _EXCEPTION_POINTERS * ex, DWORD FilteredCode);
ERRLIB_API LONG __stdcall ErrLib_CatchAll( struct _EXCEPTION_POINTERS * ex);

ERRLIB_API LPVOID __stdcall ErrLib_StrBuf_GetPointer();
ERRLIB_API LPVOID __stdcall ErrLib_ExArgs_GetPointer();

#ifdef __cplusplus
ERRLIB_API void __stdcall ErrLib_HResultToString(HRESULT hr,LPTSTR lpszFunction,WCHAR* buf);
ERRLIB_API void __stdcall ErrLib_GetHResultMessage(HRESULT hr,WCHAR* lpOutput, int cch);
}//extern "C"
#endif


//**** Helper macro functions *****

/**
 * Raises SEH exception with user-defined error message
 * 
 * @note The exception code is ERRLIB_APP_EXCEPTION (0xC0400002)
 */
#define ERRLIB_THROW(mes) {((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)mes;\
RaiseException(ERRLIB_APP_EXCEPTION,0,1,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

/**
 * Raises Win32 exception if variable is equal to the passed value
 * 
 * @param var Variable to be compared
 * @param value A value for the variable to be compared against
 * @param func The name of WINAPI function to be included in error message
 * 
 * This macro is useful to convert a WINAPI function's error code into exception to make it harder to ignore errors.
 * @note The exception code is ERRLIB_WIN32_EXCEPTION (0xC0400000)
 */
#define ERRLIB_THROW_IF_EQUAL(var,value,func) if((var)==(value)){DWORD ErrLibLocal_LastError=GetLastError();\
ErrLib_ErrorMes(L#func,ErrLibLocal_LastError,(WCHAR*)ErrLib_StrBuf_GetPointer());((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)ErrLibLocal_LastError;\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();RaiseException(ERRLIB_WIN32_EXCEPTION,0,2,(ULONG_PTR*)ErrLib_ExArgs_GetPointer());}

/**
 * Raises COM exception if passed HRESULT indicates failure
 * 
 * @param var HRESULT variable to be tested for failure
 * @param func The name of WINAPI/COM function to be included in error message
 * 
 * This macro is useful to convert a WINAPI/COM function's error code into exception to make it harder to ignore errors.
 * @note The exception code is ERRLIB_COM_EXCEPTION (0xC0400001)
 */
#define ERRLIB_THROW_IF_FAILED(var,func) if(FAILED(var)){ErrLib_HResultToString((DWORD)var,L#func,(WCHAR*)ErrLib_StrBuf_GetPointer());\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)var;((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();RaiseException(ERRLIB_COM_EXCEPTION,0,2,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

/**
 * Calls WINAPI function and raises Win32 exception if it fails. This only works for ones that return BOOL value indicating success/failure.
 * 
 * @param func the name of WINAPI function to be called
 * 
 * This macro is useful to convert a WINAPI function's error code into exception to make it harder to ignore errors.
 * @note This macro takes a variable length list of arguments to be passed into the invoked function
 * @note If the function return anything else rather then BOOL value, use ERRLIB_THROW_IF_EQUAL instead. The exception code is ERRLIB_WIN32_EXCEPTION (0xC0400000).
 */
#define ERRLIB_INVOKEAPI(func, ...) if(FALSE == func( ##__VA_ARGS__ )){DWORD ErrLibLocal_LastError=GetLastError();\
ErrLib_ErrorMes((L#func),ErrLibLocal_LastError,(WCHAR*)ErrLib_StrBuf_GetPointer());\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)ErrLibLocal_LastError;((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();\
RaiseException(ERRLIB_WIN32_EXCEPTION,0,2,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

/**
 * SEH custom `_except` block that executes handler for all exceptions
 */
#define ERRLIB_CATCH_ALL __except(ErrLib_CatchAll(GetExceptionInformation()))

/**
 * SEH custom `_except` block that executes handler for exceptions with specified code
 */
#define ERRLIB_CATCH(code) __except(ErrLib_CatchCode(GetExceptionInformation(),code))

#endif
