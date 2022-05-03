//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib Windows Event Log usage example ***

#include <stdio.h>
#include <tchar.h>
#include "ErrLib.h"

void func1(){
	ERRLIB_THROW(L"Something bad occured!") //throw custom application exception
}

void func(){
	func1();
}

int _tmain(int argc, _TCHAR* argv[])
{
	ErrLib_Initialize();

	//Register provider for Windows Event Log
	//This action requires eleveted priveleges, but it's sufficient to only call it once (fore example, after an application is installed)
	ErrLib_RegisterEventSource();
	
	//Enable logging into Windows Event Log
	ErrLib_SetParameter(ERRLIB_OUTPUT_EVENT_LOG,TRUE);
	
	__try
	{
		func();	
	}
	ERRLIB_CATCH_ALL
	{
		ErrLib_LogExceptionInfo(ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace(),TRUE);
	}
	
	getchar();
	return 0;
}

/* 
 *** Output: ***

Exception 0xc0400002: Something bad occured!
  in KERNELBASE.dll!RaiseException (C:\Windows\syswow64\KERNELBASE.dll; address: 0x7657c54f)
  in ConsoleApplication1.exe!func1 + 0x5d (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 10;)
  in ConsoleApplication1.exe!func + 0x23 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 15;)
  in ConsoleApplication1.exe!wmain + 0x85 (d:\projects\consoleapplication1\consoleapplication1.cpp; line: 27;)
  in ConsoleApplication1.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 533;)
  in ConsoleApplication1.exe!wmainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;)
  in kernel32.dll!BaseThreadInitThunk (C:\Windows\syswow64\kernel32.dll; address: 0x74af343d)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9832)
  in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9805)

 *** Event log XML: ***

 <Event xmlns="http://schemas.microsoft.com/win/2004/08/events/event">
 <System>
  <Provider Name="ConsoleApplication1.exe" /> 
  <EventID Qualifiers="49154">256</EventID> 
  <Level>2</Level> 
  <Task>1</Task> 
  <Keywords>0x80000000000000</Keywords> 
  <TimeCreated SystemTime="2018-07-09T10:36:58.000000000Z" /> 
  <EventRecordID>44085</EventRecordID> 
  <Channel>Application</Channel> 
  <Computer>mycomp-ПК</Computer> 
  <Security /> 
 </System>
 <EventData>
  <Data>Exception 0xc0400002: Something bad occured! in KERNELBASE.dll!RaiseException (C:\Windows\syswow64\KERNELBASE.dll; address: 0x7657c54f) in ConsoleApplication1.exe!func1 + 0x5d (d:\vadim\projects\consoleapplication1\consoleapplication1.cpp; line: 10;) in ConsoleApplication1.exe!func + 0x23 (d:\vadim\projects\consoleapplication1\consoleapplication1.cpp; line: 15;) in ConsoleApplication1.exe!wmain + 0x85 (d:\vadim\projects\consoleapplication1\consoleapplication1.cpp; line: 27;) in ConsoleApplication1.exe!__tmainCRTStartup + 0x199 (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 533;) in ConsoleApplication1.exe!wmainCRTStartup + 0x0d (f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c; line: 377;) in kernel32.dll!BaseThreadInitThunk (C:\Windows\syswow64\kernel32.dll; address: 0x74af343d) in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9832) in ntdll.dll!RtlInitializeExceptionChain (C:\Windows\SysWOW64\ntdll.dll; address: 0x770c9805)</Data> 
 </EventData>
 </Event>
*/

