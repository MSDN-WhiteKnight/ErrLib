//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib Win32 error handling example ***

#include <stdio.h>
#include <tchar.h>
#include <locale.h>
#include "ErrLib.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ErrLib_Initialize();	

	//Win32 error messages are based on current OS locale
	//Make sure you either set locale into correct one, or change stdout's mode into UTF-16
	setlocale(LC_ALL,"Russian"); 
		
	//Tries to open volume for direct access and read a block of data from it
	//Throws exception if WINAPI call results in error

	__try{
		HANDLE hDisk = CreateFile(L"\\\\.\\C:", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		ERRLIB_THROW_IF_EQUAL(hDisk,INVALID_HANDLE_VALUE,CreateFile)

		BYTE buf[512];
		DWORD dwRead;
		ERRLIB_INVOKEAPI(ReadFile,hDisk, buf, sizeof(buf), &dwRead, NULL) 
		wprintf(L"Read success: %d bytes read\n",(int)dwRead);
		CloseHandle(hDisk);
		
	}ERRLIB_CATCH_ALL{
		wprintf(L"Exception 0x%x: %s\n",ErrLib_Except_GetCode(),ErrLib_Except_GetMessage());
	}
	
	getchar();
	return 0;
}



