//Project: ErrLib 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
//*** ErrLib C++ exception API example ***

#include <iostream>
#include "ErrLib_CPP.h"

// Custom exception thrown when parameter value is invalid
class ValidationException : public ErrLib::Exception{
public: 
    ValidationException(std::wstring paramName){
        this->SetMsg(L"Invalid parameter value: " + paramName);
    }
};

// Function that throws exception
float CalcRectangleArea(float width, float height){
    if(width<=0.0) throw ValidationException(L"width");
    if(height<=0.0) throw ValidationException(L"height");

    return width * height;
}

void func1(){
    float s = CalcRectangleArea(0.0, 2.0);
    std::wcout << s;
    std::wcout << std::endl;
}

void func(){
    func1();
}

void threadFunction(void *param)
{
    try
    {
        func();
    }
    catch(ValidationException& ex)
    {
        // Catch exception and print diagnostic information
        std::wcout << L"ValidationException. ";
        std::wcout << ex.GetMsg();
        std::wcout << std::endl;
        std::wcout << ex.PrintStackTrace();
        std::wcout << std::endl;
    }
}

int main()
{
    ErrLib_Initialize();

    // Invoke function in background thread
    _beginthread(threadFunction, 0, NULL);

    getchar();
    return 0;
}

/* Example output:

ValidationException. Invalid parameter value: width
  in ErrLib_Demo.exe!ValidationException::ValidationException + 0x54 (c:\repos\errlib\errlib_demo\main.cpp; line: 10;)
  in ErrLib_Demo.exe!CalcRectangleArea + 0x79 (c:\repos\errlib\errlib_demo\main.cpp; line: 17;)
  in ErrLib_Demo.exe!func1 + 0x3f (c:\repos\errlib\errlib_demo\main.cpp; line: 24;)
  in ErrLib_Demo.exe!func + 0x23 (c:\repos\errlib\errlib_demo\main.cpp; line: 31;)
  in ErrLib_Demo.exe!threadFunction + 0x50 (c:\repos\errlib\errlib_demo\main.cpp; line: 38;)
  in MSVCR110D.dll!beginthread (C:\WINDOWS\SYSTEM32\MSVCR110D.dll; address: 0x51b3dd31)
  in MSVCR110D.dll!endthread (C:\WINDOWS\SYSTEM32\MSVCR110D.dll; address: 0x51b3de4e)
  in KERNEL32.DLL!BaseThreadInitThunk (C:\WINDOWS\System32\KERNEL32.DLL; address: 0x7781fa29)
  in ntdll.dll!RtlGetAppContainerNamedObjectPath (C:\WINDOWS\SYSTEM32\ntdll.dll; address: 0x77967a9e)
  in ntdll.dll!RtlGetAppContainerNamedObjectPath (C:\WINDOWS\SYSTEM32\ntdll.dll; address: 0x77967a6e)
*/
