//Project: ErrLib Test 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

#include <tchar.h>
#include <locale.h>
#include "ErrLib.h"

/********************************************************************************/

LONG WINAPI MyExceptionCallback ( struct _EXCEPTION_POINTERS * ex,LPCWSTR mes,LPCWSTR stack){

     //display error information
    fwprintf(stderr,L"Callback %s\n %s\n",mes,stack);   		

	exit(1);
    return EXCEPTION_CONTINUE_SEARCH; //crash as usual
}


void func1(){
	
        //throw std::exception();	
        //ERRLIB_THROW(L"Unknown error")

        /*int * p = NULL;	
        printf("%d",*p);*/

        WCHAR buf[MAX_PATH];
        ERRLIB_INVOKEAPI( GetModuleFileName,NULL, buf, 0);	

	/*HRESULT hr;
	hr=CoInitialize((LPVOID)1);
	ERRLIB_THROW_IF_FAILED(hr,CoInitialize)*/

        /*int a=0;
		int c;
		c = 1/a;
		printf("%d",c);*/
}

void func(){
	func1();
}

void threadFunction(void *param)
{
	/*ErrLib_InitThread();*/
    __try
    {
        func();
    }
    ERRLIB_CATCH_ALL
    {
        ErrLib_LogExceptionInfo(ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace(),TRUE);
    }
}

int _tmain(int argc, _TCHAR* argv[])
{		
	DWORD_PTR p;
	

        setlocale(LC_ALL,"Russian");
        ErrLib_Initialize();

		/*ErrLib_RegisterEventSource();		
		ErrLib_SetParameter(ERRLIB_OUTPUT_EVENT_LOG,TRUE);*/

        p = (DWORD_PTR)&func1;
        printf("0x%llx\n",(ULONGLONG)p);
		
		
		_beginthread(threadFunction, 0, NULL);
		    
		/*__try{		
			func();
		}ERRLIB_CATCH_ALL{
			printf("__except: thread %d\n", GetCurrentThreadId());
			ErrLib_LogExceptionInfo(ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace(),TRUE);
			//wprintf(L"Exception 0x%x: %s\n%s\n",ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace());

			
		}    */

        system("PAUSE");
        return 0;
}

