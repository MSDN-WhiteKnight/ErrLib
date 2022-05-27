//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

/** \file ErrLib.h
 * ErrLib public API
 */
 
/** \mainpage ErrLib documentation
 *
 * \section intro_sec Contents
 *
 * \ref ErrLib.h
 *
 * \section seealso_sec See also
 * 
 * [Source code](https://github.com/MSDN-WhiteKnight/ErrLib)
 *
 */

#ifndef ErrLib_H_INCLUDED
#define ErrLib_H_INCLUDED
#include <stdlib.h>
#include <stdio.h>

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
#define ErrLib_MessageLen 1024
#define ErrLib_StackLen 10000

#ifdef ERRLIB_EXPORTS
#define ERRLIB_API  __declspec(dllexport) 
#else
#define ERRLIB_API  __declspec(dllimport)
#pragma comment(lib, "ErrLib.lib")
#endif

// *** The following are the message definitions. ***

// MessageId: MSG_ERROR
#define MSG_ERROR                        ((DWORD)0xC0020100L)

// MessageId: MSG_WARNING
#define MSG_WARNING                      ((DWORD)0x80020101L)

// MessageId: MSG_INFORMATION
#define MSG_INFORMATION                  ((DWORD)0x40020102L)

// *** Configuration flags *** 

//Specifies the logging functions should write information into log file
#define ERRLIB_OUTPUT_LOGFILE 1

//Specifies the logging functions should write information into stderr stream
#define ERRLIB_OUTPUT_STDERR 2

//Specifies the logging functions should display information as message box
#define ERRLIB_OUTPUT_MBOX 3

//Specifies the logging functions should write information into Windows Event Log
#define ERRLIB_OUTPUT_EVENT_LOG 4

//Specifies that logging functions should invoke custom logging callback 
#define ERRLIB_OUTPUT_CUSTOM 5

#define ERRLIB_PARAM_VISUALCPPVERSION 100
#define ERRLIB_PARAM_ISDEBUGBUILD 101

// *** Typedefs *** 

//Function pointer type used as unhandled exception callback
typedef LONG  (WINAPI * ERRLIB_EXCEPTION_CALLBACK) ( struct _EXCEPTION_POINTERS *,LPCWSTR,LPCWSTR);

//Function pointer type used for custom logging targets
typedef void (WINAPI * ERRLIB_LOGGING_CALLBACK) (LPCWSTR, void*);

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
 * The **ex** parameter of the callback function points to the exception information structure. The **mes** and **stack** parameters point to strings containing human-readable error message and stack trace respectively.
 *
 * Return EXCEPTION_EXECUTE_HANDLER from callback function to execute default handler (in most cases in will crash application, invoking default Windows Error Reporting dialog window).
 * Call exit function without returning anything, if you want to avoid this behaviour and terminate application normally.
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

//Prints stack trace based on context record
ERRLIB_API void __stdcall ErrLib_PrintStack( CONTEXT* ctx , WCHAR* dest, size_t cch);

//Prints error message for specified exception
ERRLIB_API void __stdcall ErrLib_GetExceptionMessage(struct _EXCEPTION_POINTERS *ExceptionInfo, LPWSTR dest, size_t cch);

//Outputs exception information into configured log targets
ERRLIB_API void __stdcall ErrLib_LogExceptionInfo(DWORD dwExcCode,LPCWSTR lpwsMessage,LPCWSTR lpwsStackTrace, BOOL visible);

//Outputs arbitrary string into configured log targets (type can be: MSG_ERROR, MSG_WARNING or MSG_INFORMATION)
ERRLIB_API void __stdcall ErrLib_LogMessage(LPCWSTR lpwsMessage, BOOL visible, DWORD type, BOOL bIncludeStack );

//Get exception code for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API DWORD __stdcall ErrLib_Except_GetCode();

//Get error message for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetMessage();

//Get stack trace for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetStackTrace();

ERRLIB_API LONG __stdcall ErrLib_CatchCode( struct _EXCEPTION_POINTERS * ex, DWORD FilteredCode);
ERRLIB_API LONG __stdcall ErrLib_CatchAll( struct _EXCEPTION_POINTERS * ex);

ERRLIB_API LPVOID __stdcall ErrLib_StrBuf_GetPointer();
ERRLIB_API LPVOID __stdcall ErrLib_ExArgs_GetPointer();

#ifdef __cplusplus
ERRLIB_API void __stdcall ErrLib_HResultToString(HRESULT hr,LPTSTR lpszFunction,WCHAR* buf);
}//extern "C"
#endif


//**** Helper macro functions *****

//Raises custom exception
#define ERRLIB_THROW(mes) {((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)mes;\
RaiseException(ERRLIB_APP_EXCEPTION,0,1,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

//Raises Win32 exception if variable is equal to the passed value
#define ERRLIB_THROW_IF_EQUAL(var,value,func) if((var)==(value)){DWORD ErrLibLocal_LastError=GetLastError();\
ErrLib_ErrorMes(L#func,ErrLibLocal_LastError,(WCHAR*)ErrLib_StrBuf_GetPointer());((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)ErrLibLocal_LastError;\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();RaiseException(ERRLIB_WIN32_EXCEPTION,0,2,(ULONG_PTR*)ErrLib_ExArgs_GetPointer());}

//Raises COM exception if passed HRESULT indicates failure
#define ERRLIB_THROW_IF_FAILED(var,func) if(FAILED(var)){ErrLib_HResultToString((DWORD)var,L#func,(WCHAR*)ErrLib_StrBuf_GetPointer());\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)var;((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();RaiseException(ERRLIB_COM_EXCEPTION,0,2,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

//Calls WINAPI function and raises Win32 exception if it fails
#define ERRLIB_INVOKEAPI(func, ...) if(FALSE == func( ##__VA_ARGS__ )){DWORD ErrLibLocal_LastError=GetLastError();\
ErrLib_ErrorMes((L#func),ErrLibLocal_LastError,(WCHAR*)ErrLib_StrBuf_GetPointer());\
((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[0]=(ULONG_PTR)ErrLibLocal_LastError;((ULONG_PTR*)ErrLib_ExArgs_GetPointer())[1]=(ULONG_PTR)ErrLib_StrBuf_GetPointer();\
RaiseException(ERRLIB_WIN32_EXCEPTION,0,2,((ULONG_PTR*)ErrLib_ExArgs_GetPointer()));}

//SEH Catch block that executes handler for all exceptions
#define ERRLIB_CATCH_ALL __except(ErrLib_CatchAll(GetExceptionInformation()))

//SEH Catch block that executes handler for exceptions with specified code
#define ERRLIB_CATCH(code) __except(ErrLib_CatchCode(GetExceptionInformation(),code))

#endif
