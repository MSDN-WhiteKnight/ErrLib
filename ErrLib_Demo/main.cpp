//Project: ErrLib Test 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

#include <stdio.h>
#include "ErrLib.h"
#include "ErrLib_CPP.h"

/*******************************************************************************/

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

void ErrLibCppDemo(){
    try{
        func();
    }
    catch(ErrLib::Exception& e){
        WCHAR buf[1024]=L"";
        fputws(L"ErrLib::Exception\r\n", stdout);

        e.GetMessageText(buf,1024);
        fputws(buf, stdout);
        fputws(L"\r\n", stdout);

        CONTEXT ctx;
        e.GetContext(&ctx);
        ErrLib_PrintStack(&ctx, buf, 1024);
        fputws(buf, stdout);
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
