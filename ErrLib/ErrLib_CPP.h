//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

/** \file ErrLib_CPP.h
 * ErrLib public C++ API
 */

#ifndef ERRLIB_CPP_H_INCLUDED
#define ERRLIB_CPP_H_INCLUDED
#include "ErrLib.h"
#include <string>

const DWORD ERRLIB_CPP_EXCEPTION = 0xC0400002;

#ifdef __cplusplus
/**
 * ErrLib public C++ API 
 */
namespace ErrLib{

/**
 * Provides a base class for C++ exceptions that supports capturing a stack trace in the moment when exception 
 * is thrown and logging the exception information. 
 */
class Exception : public std::exception{
private:
    CONTEXT _context;
    std::wstring _msg;
    std::wstring _stack;
    DWORD _code;
    void* _data;

    std::wstring PrintStackTraceImpl(){
        WCHAR buf[ErrLib_StackLen]=L"";
        ErrLib_PrintStack(&_context, buf, ErrLib_StackLen);
        return std::wstring(buf);
    }

protected: 

    void SetMsg(const std::wstring& msg){_msg=msg;}

    void SetCode(DWORD code){_code=code;}

    void SetData(void* data){_data=data;}

public:

    /**
     * Creates a new exception using the default empty error message
     */
    Exception():_msg(L""),_code(ERRLIB_CPP_EXCEPTION),_data(nullptr){
        RtlCaptureContext(&_context);
        _stack = this->PrintStackTraceImpl();
    }

    /**
     * Creates a new exception using the specified error message
     */
    Exception(const std::wstring& message):_msg(message),_code(ERRLIB_CPP_EXCEPTION),_data(nullptr){
        RtlCaptureContext(&_context);
        _stack = this->PrintStackTraceImpl();
    }

    /**
     * Creates a new exception using the specified error message, error code and additional data
     */
    Exception(const std::wstring& message, DWORD code, void* data):_msg(message),_code(code),_data(data){
        RtlCaptureContext(&_context);
        _stack = this->PrintStackTraceImpl();
    }
    
    /**
     * Gets the error message associated with this exception as a C++ wstring
     */
    std::wstring GetMsg(){return _msg;}

    /**
     * Gets the error code associated with this exception
     * @note When error code was not specified when creating exception, the default value DWORD ERRLIB_CPP_EXCEPTION (0xC0400002) is used.
     */
    DWORD GetCode(){return _code;}

    /**
     * Gets the additional user-defined data associated with this exception
     */
    void* GetData(){return _data;}

    /**
     * Gets the error message associated with this exception as a C wide-character string
     * @param pOutput The pointer to the caller-allocated wide character array that will be filled with error message text on output.
     * @param cch The maximum amount of characters that can be put into the array pointed by **pOutput** parameter.
     */
    void GetMessageText(WCHAR* pOutput, int cch){
        const WCHAR* pChars = _msg.c_str();
        wcscpy_s(pOutput, cch, pChars);
    }

    /**
     * Gets the processor context in the moment this exception was thrown
     * @param pOutput The pointer to the caller-allocated buffer to store the CONTEXT structure. Must be at least sizeof(CONTEXT) bytes.
     */
    void GetContext(CONTEXT* pOutput){
        memcpy(pOutput, &_context, sizeof(CONTEXT));
    }

    /**
     * Gets the stack trace in the moment this exception was thrown as a C wide-character string
     * @param pOutput The pointer to the caller-allocated wide character array that will be filled with stack trace text on output.
     * @param cch The maximum amount of characters that can be put into the array pointed by **pOutput** parameter.
     */
    void PrintStackTrace(WCHAR* pOutput, int cch){
        const WCHAR* p = this->_stack.c_str();
        StringCchCopy(pOutput, cch, p);
    }

    /**
     * Gets the stack trace in the moment this exception was thrown as a C++ wstring
     */
    std::wstring PrintStackTrace(){
        return _stack;
    }

    /**
     * Outputs the exception information into configured log targets
     * @param visible Pass `true` if you want to use ERRLIB_OUTPUT_STDERR/ERRLIB_OUTPUT_MBOX logging targets (if they are enabled by configuration flags), `false` otherwise
     * 
     * This method outputs information into one or more logging targets, configured using ErrLib_SetParameter function.
     * It only outputs information into the stderr stream and message box if the **visible** parameter is `true` (and if respective flags are enabled).
     * By default, the enabled logging targets are log file and stderr stream. 
     * @note When outputting information in message box, stack trace is not included.
     */
    void Log(bool visible){
        BOOL bVisible;

        if (visible) bVisible = TRUE;
        else bVisible = FALSE;

        ErrLib_LogExceptionInfo(_code, _msg.c_str(), this->PrintStackTrace().c_str(), bVisible);
    }
};

/**
 * Represents an error in Windows native API function
 */
class WinapiException : public Exception{
public: 
    WinapiException(DWORD code,const std::wstring& msg){
        this->SetCode(code);
        this->SetMsg(msg);
    }

    /**
     * Creates a new WinapiException using the error code and message from the last WINAPI error
     * @param localized The value indicating whether the exception message should use the current OS locale
     * @note If the **localized** parameter is `true`, the message is in language of the current OS locale. Otherwise, it is always in English.
     */
    static WinapiException FromLastError(bool localized){
        DWORD code = GetLastError();
        BOOL fLocalized;
        WCHAR buf[ErrLib_MessageLen]=L"";

        if(localized) fLocalized = TRUE;
        else fLocalized = FALSE;
        
        ErrLib_GetWinapiErrorMessage(code, fLocalized, buf, ErrLib_MessageLen);
        return WinapiException(code, buf);
    }
};

/**
 * Represents an error in Windows COM API function or method defined by the integer HRESULT code
 */
class ComException : public Exception{
public: 
    ComException(HRESULT hr,const std::wstring& msg){
        this->SetCode((DWORD)hr);
        this->SetMsg(msg);
    }

    /**
     * Creates a new ComException using the error code and message from the specified HRESULT
     * @param hr HRESULT value that would be used as error code
     */
    static ComException FromHResult(HRESULT hr){
        WCHAR buf[ErrLib_MessageLen]=L"";
        
        ErrLib_GetHResultMessage(hr, buf, ErrLib_MessageLen);
        return ComException(hr, buf);
    }
};

}
#endif //__cplusplus
#endif
