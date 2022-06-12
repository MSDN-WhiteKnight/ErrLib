//Project: ErrLib Test 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

#include <stdio.h>
#include "ErrLib.h"

/********************************************************************************/

LONG WINAPI MyExceptionCallback (struct _EXCEPTION_POINTERS* ex, LPCWSTR mes, LPCWSTR stack){
    //display error information
    fwprintf(stderr,L"Callback %s\n %s\n",mes,stack);   		

    exit(1);
    return EXCEPTION_CONTINUE_SEARCH; //crash as usual
}

void func1(){
    int a=0;
    int c;
    c = 1/a;
    printf("%d",c);
}

void func(){
    func1();
}

void threadFunction(void *param)
{
    __try
    {
        func();
    }
    ERRLIB_CATCH_ALL
    {
        ErrLib_LogExceptionInfo(ErrLib_Except_GetCode(),ErrLib_Except_GetMessage(),ErrLib_Except_GetStackTrace(),TRUE);
    }
}

int main()
{		
    DWORD_PTR p;
    ErrLib_Initialize();
	
    p = (DWORD_PTR)&func1;
    printf("0x%llx\n",(ULONGLONG)p);

    _beginthread(threadFunction, 0, NULL);
 
    getchar();
    return 0;
}
