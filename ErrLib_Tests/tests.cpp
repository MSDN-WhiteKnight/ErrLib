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

class MyException : public ErrLib::Exception{
public: MyException(){this->SetMsg(L"Test exception subclass");}
};

void foo(){throw MyException();}

void bar(){foo();}

ERRLIB_STACK_TRACE StackTraceTestFunc(){
    CONTEXT ctx;
    RtlCaptureContext(&ctx);
    ERRLIB_STACK_TRACE stackTrace = ErrLib_GetStackTrace(&ctx);
    return stackTrace;
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

void Assert_Contains(const WCHAR* str, const WCHAR* substr){
    const WCHAR* match = wcsstr(str, substr);
    Assert::IsTrue(match != NULL);
}

void Assert_ContainsSymbol(const ERRLIB_STACK_TRACE* pStack, const WCHAR* symbol){
    bool found = false;
    WCHAR frameSymbol[MAX_SYM_NAME]=L"";
    ERRLIB_STACK_FRAME frame;

    for(int i=0;i<ErrLib_ST_GetFramesCount(pStack);i++){
        
        ErrLib_ST_GetFrame(pStack,i,&frame);
        ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_NAME, frameSymbol, MAX_SYM_NAME);

        if(wcscmp(frameSymbol, symbol) == 0) {
            found = true;
            break;
        }
    }

    Assert::IsTrue(found);
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

        TEST_METHOD(Test_Exception_Derived)
        {
            MyException exc;
            Assert::AreEqual(ERRLIB_CPP_EXCEPTION, exc.GetCode());
            Assert::AreEqual<std::wstring>(L"Test exception subclass", exc.GetMsg());
            Assert::AreEqual<void*>(nullptr, exc.GetData());

            WCHAR buf[ErrLib_MessageLen];
            exc.GetMessageText(buf,ErrLib_MessageLen);
            Assert::AreEqual(L"Test exception subclass", buf);
        }

        TEST_METHOD(Test_Cpp_Catch_Derived) 
        {
            int code=0;
            void* data=nullptr;
            WCHAR mes[BUFFER_SIZE]=L"";
            WCHAR stack[ErrLib_StackLen]=L"";
            WCHAR* match;
            std::wstring stackStr;

            try {
                bar();
            }
            catch(MyException& e) {
                code = e.GetCode();
                data = e.GetData();
                e.GetMessageText(mes, BUFFER_SIZE);
                e.PrintStackTrace(stack, ErrLib_StackLen);
                stackStr = e.PrintStackTrace();

                Assert::AreEqual<std::wstring>(L"Test exception subclass",e.GetMsg());
            }

            Assert::AreEqual<DWORD>(ERRLIB_CPP_EXCEPTION, code);
            Assert::AreEqual((UINT_PTR)nullptr, (UINT_PTR)data);
            Assert::AreEqual(L"Test exception subclass", mes);
            
            // Stack trace
            Assert::IsTrue(wcslen(stack)>20);
            Assert::AreEqual<std::wstring>(stackStr,stack);

            if(DEBUG_BUILD){
                match = wcsstr(stack,L"ErrLib_Tests.dll!foo");
                Assert::IsTrue(match!=NULL);

                match = wcsstr(stack,L"ErrLib_Tests.dll!bar");
                Assert::IsTrue(match!=NULL);
            
                match = wcsstr(stack,L"ErrLib_Tests.dll!ErrLib_Tests::Tests::Test_Cpp_Catch_Derived");
                Assert::IsTrue(match!=NULL);            

                match = wcsstr(stack,L"tests.cpp;");
                Assert::IsTrue(match!=NULL);
            }
        }

        TEST_METHOD(Test_WinapiException_FromLastError)
        {
            CloseHandle(NULL);
            ErrLib::WinapiException exc=ErrLib::WinapiException::FromLastError(false);
            
            Assert::AreEqual<DWORD>(6, exc.GetCode());
            Assert::AreEqual<std::wstring>(L"The handle is invalid.\r\n", exc.GetMsg());
        }

        TEST_METHOD(Test_WinapiException_FromLastError_Localized)
        {
            CloseHandle(NULL);
            ErrLib::WinapiException exc=ErrLib::WinapiException::FromLastError(true);
            
            Assert::AreEqual<DWORD>(6, exc.GetCode());

            // Just verify that it does not crash and returns non-empty string. The exact message depends on current language.
            Assert::IsTrue(exc.GetMsg().length() > 0);
        }

        TEST_METHOD(Test_Exception_FromHResult)
        {
            HRESULT hr = E_OUTOFMEMORY;
            ErrLib::ComException exc=ErrLib::ComException::FromHResult(hr);
            
            Assert::AreEqual<DWORD>(E_OUTOFMEMORY, exc.GetCode());

            // Just verify that it does not crash and returns non-empty string. The exact message depends on current language.
            Assert::IsTrue(exc.GetMsg().length() > 0);
        }

        TEST_METHOD(Test_GetStackTrace){
            ERRLIB_STACK_TRACE stackTrace = StackTraceTestFunc();
            ERRLIB_STACK_FRAME firstFrame;
            WCHAR buf[MAX_PATH]=L"";
            int nChars;
            BOOL res = ErrLib_ST_GetFrame(&stackTrace, 0, &firstFrame);
            
            Assert::IsTrue(res != FALSE);
            Assert::IsTrue(ErrLib_ST_GetFramesCount(&stackTrace) > 1);
            Assert::IsTrue(stackTrace.capacity > stackTrace.count);
            Assert::IsTrue(stackTrace.isOnHeap != FALSE);
            Assert::IsTrue(stackTrace.data != NULL);
            Assert::IsTrue(ErrLib_ST_GetAddress(&firstFrame) != 0x0);

            if(DEBUG_BUILD){
                // x86 stack trace does not contain the direct caller for some reason, so we assert
                // on the second frame to cover both cases
                Assert_ContainsSymbol(&stackTrace, L"ErrLib_Tests::Tests::Test_GetStackTrace");

                nChars = ErrLib_ST_GetStringProperty(&firstFrame, ERRLIB_SYMBOL_MODULE, buf, MAX_PATH);
                Assert_Contains(buf, L"ErrLib_Tests.dll");
                Assert::AreEqual<int>(wcslen(buf)+1, nChars);

                nChars = ErrLib_ST_GetStringProperty(&firstFrame, ERRLIB_SYMBOL_SOURCE, buf, MAX_PATH);
                Assert_Contains(buf, L"tests.cpp");
                Assert::AreEqual<int>(wcslen(buf)+1, nChars);
            }

            ErrLib_FreeStackTrace(&stackTrace);
            Assert::AreEqual(0, ErrLib_ST_GetFramesCount(&stackTrace));
            Assert::AreEqual(0, stackTrace.capacity);
            Assert::AreEqual<void*>(NULL, stackTrace.data);
        }

        TEST_METHOD(Test_StackFrame){
            ERRLIB_STACK_FRAME frame;
            WCHAR buf[MAX_SYM_NAME]=L"";
            int nChars=0;

            const WCHAR* ExampleSymbol = L"ExampleSymbolName";
            const WCHAR* ExampleModule = L"module.dll";
            const WCHAR* ExampleSource = L"example.cpp";

            StringCchCopy(frame.symbol, MAX_SYM_NAME, ExampleSymbol);
            StringCchCopy(frame.module, MAX_PATH, ExampleModule);
            StringCchCopy(frame.src_file, MAX_PATH, ExampleSource);

            //buffer is too short
            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_NAME, buf, 3);
            Assert::AreEqual<int>(wcslen(ExampleSymbol)+1, nChars);
            Assert::AreEqual(L"Ex", buf);

            memset(buf, 0, sizeof(buf));
            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_MODULE, buf, 1);
            Assert::AreEqual<int>(wcslen(ExampleModule)+1, nChars);
            Assert::AreEqual(L"", buf);

            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_SOURCE, buf, 2);
            Assert::AreEqual<int>(wcslen(ExampleSource)+1, nChars);
            Assert::AreEqual(L"e", buf);

            //buffer is long enough
            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_NAME, buf, MAX_SYM_NAME);
            Assert::AreEqual<int>(wcslen(ExampleSymbol)+1, nChars);
            Assert::AreEqual(ExampleSymbol, buf);

            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_MODULE, buf, MAX_SYM_NAME);
            Assert::AreEqual<int>(wcslen(ExampleModule)+1, nChars);
            Assert::AreEqual(ExampleModule, buf);

            nChars = ErrLib_ST_GetStringProperty(&frame, ERRLIB_SYMBOL_SOURCE, buf, MAX_SYM_NAME);
            Assert::AreEqual<int>(wcslen(ExampleSource)+1, nChars);
            Assert::AreEqual(ExampleSource, buf);
        }
    };
}
