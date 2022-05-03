#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include "CppUnitTest.h"
#include "ErrLib.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#if defined(_DEBUG)
const bool DEBUG_BUILD = true;
#else
const bool DEBUG_BUILD = false;
#endif

void func1(){
	int a=0;
	int c;
	c = 1/a;
	printf("%d",c);
}

void func(){
	func1();
}

namespace ErrLib_Tests
{
    TEST_CLASS(Tests) 
    {
    public:
		
        Tests() 
        {
            ErrLib_Initialize();
        }

        TEST_METHOD(Test_Errlib_Catch) 
        {
            DWORD code=0;
            WCHAR message[1000]=L"";
            WCHAR stack[10000]=L"";
            WCHAR* match;

            __try {
                func();
            }
            ERRLIB_CATCH(EXCEPTION_INT_DIVIDE_BY_ZERO) {
                code = ErrLib_Except_GetCode();
                wcscpy(message, ErrLib_Except_GetMessage());
                wcscpy(stack, ErrLib_Except_GetStackTrace());                
            }

            Assert::AreEqual(0xc0000094, (UINT)code);
            Assert::AreEqual(L"Integer division by zero",message);
            Assert::IsTrue(wcslen(stack)>20);

            //Stack trace
            if(DEBUG_BUILD){
                match = wcsstr(stack,L"ErrLib_Tests.dll!func1");
                Assert::IsTrue(match!=NULL);
            
                match = wcsstr(stack,L"ErrLib_Tests.dll!ErrLib_Tests::Tests::Test_Errlib_Catch");
                Assert::IsTrue(match!=NULL);            

                match = wcsstr(stack,L"tests.cpp;");
                Assert::IsTrue(match!=NULL);
            }
        }
    };
}