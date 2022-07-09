//Project: ErrLib Demo 
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

#include <stdio.h>
#include "ErrLib.h"
#include "ErrLib_CPP.h"

// Function that throws exception when parameter value is invalid
float CalcRectangleArea(float width, float height){
    if(width<=0.0) throw ErrLib::Exception(L"Width must be positive");
    if(height<=0.0) throw ErrLib::Exception(L"Height must be positive");

    return width * height;
}

void func1(){
    float s = CalcRectangleArea(0.0, 2.0);
    wprintf(L"%f\n", s);
}

void func(){
    try
    {
        func1();
    }
    catch(ErrLib::Exception& ex)
    {
        // Catch exception and print diagnostic information
        wprintf(L"Exception: %s\n%s", ex.GetMsg().c_str(), ex.PrintStackTrace().c_str());
    }
}

int main()
{
    ErrLib_Initialize();

    func();

    getchar();
    return 0;
}
