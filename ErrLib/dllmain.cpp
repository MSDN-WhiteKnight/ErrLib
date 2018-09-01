//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

// dllmain.cpp: определяет точку входа для приложения DLL.
#define ERRLIB_EXPORTS
#include "ErrLib.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: ErrLib_InitTLS(); ErrLib_InitThread();break;

	case DLL_THREAD_ATTACH: ErrLib_InitThread();break;

	case DLL_THREAD_DETACH: ErrLib_FreeThread();break;

	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

