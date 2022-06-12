//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
#ifndef ERRLIB_CPP_H_INCLUDED
#define ERRLIB_CPP_H_INCLUDED
#include "ErrLib.h"
#include <string>

const DWORD ERRLIB_CPP_EXCEPTION = 0xC0400002;

#ifdef __cplusplus
namespace ErrLib{

class Exception : public std::exception{
private:
    CONTEXT _context;
    std::wstring _msg;
    DWORD _code;
    void* _data;

public:

    Exception():_msg(L""),_code(ERRLIB_CPP_EXCEPTION),_data(nullptr){
        RtlCaptureContext(&_context);
    }

    Exception(const std::wstring& message):_msg(message),_code(ERRLIB_CPP_EXCEPTION),_data(nullptr){
        RtlCaptureContext(&_context);
    }

    Exception(const std::wstring& message, DWORD code, void* data):_msg(message),_code(code),_data(data){
        RtlCaptureContext(&_context);
    }
    
    std::wstring GetMsg(){return _msg;}

    int GetCode(){return _code;}

    void* GetData(){return _data;}

    void GetMessageText(WCHAR* pOutput, int cch){
        const WCHAR* pChars = _msg.c_str();
        wcscpy_s(pOutput, cch, pChars);
    }

    void GetContext(CONTEXT* pOutput){
        memcpy(pOutput, &_context, sizeof(CONTEXT));
    }

    void PrintStackTrace(WCHAR* pOutput, int cch){
        ErrLib_PrintStack(&_context, pOutput, cch);
    }

    std::wstring PrintStackTrace(){
        WCHAR buf[ErrLib_StackLen]=L"";
        this->PrintStackTrace(buf, ErrLib_StackLen);
        return std::wstring(buf);
    }

    void Log(bool visible){
        BOOL bVisible;

        if (visible) bVisible = TRUE;
        else bVisible = FALSE;

        ErrLib_LogExceptionInfo(_code, _msg.c_str(), this->PrintStackTrace().c_str(), bVisible);
    }
};

}
#endif //__cplusplus
#endif
