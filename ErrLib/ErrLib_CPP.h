//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
#ifndef ERRLIB_CPP_H_INCLUDED
#define ERRLIB_CPP_H_INCLUDED
#include "ErrLib.h"
#include <string>

namespace ErrLib{

const int CppExceptionCode = 1;

class Exception : public std::exception{
private:
    CONTEXT _context;
    std::wstring _msg;
    int _code;
    void* _data;

public:

    Exception():_msg(L""),_code(CppExceptionCode),_data(nullptr){
        RtlCaptureContext(&_context);
    }

    Exception(const std::wstring& message):_msg(message),_code(CppExceptionCode),_data(nullptr){
        RtlCaptureContext(&_context);
    }

    Exception(const std::wstring& message, int code, void* data):_msg(message),_code(code),_data(data){
        RtlCaptureContext(&_context);
    }
    
    std::wstring GetMessage(){return _msg;}

    int GetCode(){return _code;}

    void* GetData(){return _data;}

    void GetMessageText(WCHAR* pOutput, size_t cch){
        const WCHAR* pChars = _msg.c_str();
        wcscpy_s(pOutput, cch, pChars);
    }

    void GetContext(CONTEXT* pOutput){
        memcpy(pOutput, &_context, sizeof(CONTEXT));
    }
};

}
#endif
