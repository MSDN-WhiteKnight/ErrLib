#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include "CppUnitTest.h"
#include "ErrLib.h"
#include "ErrLib_CPP.h"
#include "vcdefs.h"

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

void cpp_func1(){
    throw ErrLib::Exception(L"Error occured", 123, nullptr);
}

void cpp_func(){
    cpp_func1();
}

const WCHAR logname[] = L"errlib.log";
const int BUFFER_SIZE = 5000;
WCHAR buf[BUFFER_SIZE]=L"";

void WINAPI MyLoggingCallback(LPCWSTR pStr, void* pExtraInfo){
    wcscpy(buf, pStr);
}

void FileReadAllLines(const WCHAR* logname, WCHAR* pOutput, int cch){
    //read content of log file
    WCHAR* p=NULL;
    WCHAR buf[BUFFER_SIZE]=L"";
    FILE* fp;
    int n=0;
    int len;
    fp = _wfopen(logname, L"rt");
    wcscpy_s(pOutput,cch,L"");

    while(true){
        p = fgetws(buf,BUFFER_SIZE,fp);
        if(p==NULL) break;

        len = wcslen(buf);
        if(n+len>=cch) break;

        wcscat_s(pOutput,cch,buf);
        n+=len;
    }

    fclose(fp);
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
            
            // Stack trace
            Assert::IsTrue(wcslen(stack)>20);

            if(DEBUG_BUILD){
                match = wcsstr(stack,L"ErrLib_Tests.dll!func1");
                Assert::IsTrue(match!=NULL);
            
                match = wcsstr(stack,L"ErrLib_Tests.dll!ErrLib_Tests::Tests::Test_Errlib_Catch");
                Assert::IsTrue(match!=NULL);            

                match = wcsstr(stack,L"tests.cpp;");
                Assert::IsTrue(match!=NULL);
            }
        }

        TEST_METHOD(Test_LogMessage)
        {
            WCHAR text[BUFFER_SIZE]=L"";
            WCHAR* match = NULL;

            //write message to log file
            DeleteFile(logname);
            ErrLib_SetLogFilePath(logname);
            ErrLib_LogMessage(L"Quick brown fox", FALSE, MSG_INFORMATION, FALSE);

            //read content of log file
            FileReadAllLines(logname, text, BUFFER_SIZE);

            //verify content
            match = wcsstr(text, L"Quick brown fox");
            Assert::IsTrue(match != NULL);
        }

        TEST_METHOD(Test_LogExceptionInfo)
        {
            WCHAR text[BUFFER_SIZE]=L"";
            WCHAR* match = NULL;

            //write exception to log file
            DeleteFile(logname);
            ErrLib_SetLogFilePath(logname);
            ErrLib_LogExceptionInfo(1,L"ErrLib test exception",L"",FALSE);

            //read content of log file
            FileReadAllLines(logname, text, BUFFER_SIZE);

            //verify content
            match = wcsstr(text, L"Exception 0x1: ErrLib test exception");
            Assert::IsTrue(match != NULL);
        }

        TEST_METHOD(Test_Errlib_CustomTarget)
        {
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)TRUE);
            ErrLib_SetLoggingCallback(MyLoggingCallback);
            ZeroMemory(buf, sizeof(buf));
            ErrLib_LogMessage(L"Test_Errlib_CustomTarget", FALSE, MSG_INFORMATION, FALSE);
            Assert::AreEqual(L"Test_Errlib_CustomTarget", buf);
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)FALSE);
        }

        TEST_METHOD(Test_CustomTarget_LogExceptionInfo)
        {
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)TRUE);
            ErrLib_SetLoggingCallback(MyLoggingCallback);
            ZeroMemory(buf, sizeof(buf));
            ErrLib_LogExceptionInfo(0x0A,L"Test_CustomTarget_LogExceptionInfo",L"",FALSE);
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)FALSE);
                        
            WCHAR* match = wcsstr(buf, L"Exception 0xa: Test_CustomTarget_LogExceptionInfo");
            Assert::IsTrue(match != NULL);
        }

        TEST_METHOD(Test_GetWinapiErrorMessage){
            WCHAR buf[BUFFER_SIZE]=L"";
            DWORD res = ErrLib_GetWinapiErrorMessage(2, FALSE, buf, BUFFER_SIZE);
            
            Assert::IsTrue(res>0);
            Assert::AreEqual(L"The system cannot find the file specified.\r\n",buf);
        }

        TEST_METHOD(Test_GetWinapiErrorMessage_Null){
            DWORD res = ErrLib_GetWinapiErrorMessage(2, FALSE, NULL, 0);
            Assert::AreEqual<DWORD>(0,res);
        }

        TEST_METHOD(Test_Cpp_Catch) 
        {
            int code=0;
            void* data=nullptr;
            WCHAR mes[BUFFER_SIZE]=L"";
            WCHAR stack[ErrLib_StackLen]=L"";
            WCHAR* match;
            std::wstring stackStr;

            try {
                cpp_func();
            }
            catch(ErrLib::Exception& e) {
                code = e.GetCode();
                data = e.GetData();
                e.GetMessageText(mes, BUFFER_SIZE);
                e.PrintStackTrace(stack, ErrLib_StackLen);
                stackStr = e.PrintStackTrace();

                Assert::AreEqual<std::wstring>(L"Error occured",e.GetMsg());
            }

            Assert::AreEqual(123, code);
            Assert::AreEqual((UINT_PTR)nullptr, (UINT_PTR)data);
            Assert::AreEqual(L"Error occured", mes);
            
            // Stack trace
            Assert::IsTrue(wcslen(stack)>20);
            Assert::AreEqual<std::wstring>(stackStr,stack);

            if(DEBUG_BUILD){
                match = wcsstr(stack,L"ErrLib_Tests.dll!cpp_func1");
                Assert::IsTrue(match!=NULL);
            
                match = wcsstr(stack,L"ErrLib_Tests.dll!ErrLib_Tests::Tests::Test_Cpp_Catch");
                Assert::IsTrue(match!=NULL);            

                match = wcsstr(stack,L"tests.cpp;");
                Assert::IsTrue(match!=NULL);
            }
        }

        TEST_METHOD(Test_Cpp_LogExceptionInfo)
        {
            WCHAR text[BUFFER_SIZE]=L"";
            WCHAR* match = NULL;

            //write exception to log file
            DeleteFile(logname);
            ErrLib_SetLogFilePath(logname);
            ErrLib::Exception exc(L"ErrLib test exception",(DWORD)1,(void*)nullptr);
            exc.Log(false);

            //read content of log file
            FileReadAllLines(logname, text, BUFFER_SIZE);

            //verify content
            match = wcsstr(text, L"Exception 0x1: ErrLib test exception");
            Assert::IsTrue(match != NULL);
        }

        TEST_METHOD(Test_Cpp_CustomLogTarget)
        {
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)TRUE);
            ErrLib_SetLoggingCallback(MyLoggingCallback);
            ZeroMemory(buf, sizeof(buf));

            ErrLib::Exception exc(L"Test_Cpp_CustomLogTarget",(DWORD)0x0B,(void*)nullptr);
            exc.Log(false);
            ErrLib_SetParameter(ERRLIB_OUTPUT_CUSTOM, (UINT_PTR)FALSE);
                        
            WCHAR* match = wcsstr(buf, L"Exception 0xb: Test_Cpp_CustomLogTarget");
            Assert::IsTrue(match != NULL);
        }

        TEST_METHOD(Test_Exception_DefaultParams)
        {
            ErrLib::Exception exc;
            Assert::AreEqual(ERRLIB_CPP_EXCEPTION, exc.GetCode());
            Assert::AreEqual<std::wstring>(L"", exc.GetMsg());
            Assert::AreEqual<void*>(nullptr, exc.GetData());

            WCHAR buf[ErrLib_MessageLen];
            exc.GetMessageText(buf,ErrLib_MessageLen);
            Assert::AreEqual(L"", buf);
        }

        TEST_METHOD(Test_Exception_FromLastWinapiError)
        {
            CloseHandle(NULL);
            ErrLib::Exception exc=ErrLib::Exception::FromLastWinapiError(false);
            
            Assert::AreEqual<DWORD>(6, exc.GetCode());
            Assert::AreEqual<std::wstring>(L"The handle is invalid.\r\n", exc.GetMsg());
        }

        TEST_METHOD(Test_Exception_FromLastWinapiError_Localized)
        {
            CloseHandle(NULL);
            ErrLib::Exception exc=ErrLib::Exception::FromLastWinapiError(true);
            
            Assert::AreEqual<DWORD>(6, exc.GetCode());

            // Just verify that it does not crash and returns non-empty string. The exact message depends on current language.
            Assert::IsTrue(exc.GetMsg().length() > 0);
        }
    };
}