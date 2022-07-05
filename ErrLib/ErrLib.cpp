//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)

#define ERRLIB_EXPORTS
#include "ErrLib.h"
#include "event.h"
#include "vcdefs.h"

// *** internal functions *** 

LONG WINAPI ErrLib_MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo);
LONG WINAPI ErrLib_DefaultExceptionCallback ( struct _EXCEPTION_POINTERS * ex,LPCWSTR mes,LPCWSTR stack);

// *** variables ***

CRITICAL_SECTION ErrLib_DbgHlpSync={0}; //DbgHlp functions syncronization object
CRITICAL_SECTION ErrLib_LogSync={0}; //ErrLib logging functions syncronization object
ERRLIB_EXCEPTION_CALLBACK volatile ErrLib_pCurrentExceptionCallback = NULL; //Pointer to a user function called on unhandled exception
ERRLIB_LOGGING_CALLBACK volatile ErrLib_pCurrentLoggingCallback = NULL; //Pointer to a user function used as a custom logging target
LPTOP_LEVEL_EXCEPTION_FILTER volatile ErrLib_pPreviousExceptionFilter = NULL; //Unhandled exception filter which was set before ErrLib
WCHAR ErrLib_LogFilePath[MAX_PATH]={0}; //A path to the current error log file
BOOL volatile ErrLib_fOutputLogFile = TRUE; //Configuration flag: Output errors to log file
BOOL volatile ErrLib_fOutputStderr = TRUE; //Configuration flag: Output errors to stderr stream
BOOL volatile ErrLib_fOutputMbox = FALSE; //Configuration flag: Output errors as message box
BOOL volatile ErrLib_fOutputEventLog = FALSE; //Configuration flag: Output errors into Windows Event Log
BOOL volatile ErrLib_fOutputCustom = FALSE; //Configuration flag: Output errors into custom target

int ErrLib_VisualCppRtVersion = MSVC_VERSION;
BOOL ErrLib_fDebugBuild = FALSE;

DWORD volatile ErrLib_tlsiLastExceptionCode = 0;
DWORD volatile ErrLib_tlsiLastExceptionMessage = 0;
DWORD volatile ErrLib_tlsiLastExceptionStack = 0;
DWORD volatile ErrLib_tlsiStrBuf = 0;
DWORD volatile ErrLib_tlsiExArgs = 0;

// TLS ID: Last exception stack trace
DWORD volatile ErrLib_tlsiStackTrace = 0;

#ifdef __cplusplus
extern "C" {
#endif

ERRLIB_STACK_TRACE StackTrace_Alloc(int capacity);
ERRLIB_STACK_TRACE StackTrace_Copy(const ERRLIB_STACK_TRACE* pInput);

BOOL IsOnVisualCpp2015OrAbove() {

    if (MSVC_VERSION >= VISUAL_STUDIO_V2015) return TRUE;

    return ErrLib_VisualCppRtVersion >= VISUAL_STUDIO_V2015;
}

BOOL IsStackTraceDisabled() {
    return FALSE;
}

/* Pointer getters*/

ERRLIB_API LPVOID __stdcall ErrLib_StrBuf_GetPointer(){
	return TlsGetValue(ErrLib_tlsiStrBuf);
	//return ErrLib_StrBuf;
}

ERRLIB_API LPVOID __stdcall ErrLib_ExArgs_GetPointer(){
	return TlsGetValue(ErrLib_tlsiExArgs);
	//return ErrLib_ExArgs;
}

LPVOID ErrLib_LastExceptionCode_GetPointer(){
	return TlsGetValue(ErrLib_tlsiLastExceptionCode);
	//return &ErrLib_LastExceptionCode;
}

//Get error message for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetMessage(){
	return (LPWSTR)TlsGetValue(ErrLib_tlsiLastExceptionMessage);
	//return ErrLib_LastExceptionMessage;
}

//Get stack trace for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API LPWSTR __stdcall ErrLib_Except_GetStackTrace(){
	return (LPWSTR)TlsGetValue(ErrLib_tlsiLastExceptionStack);
	//return ErrLib_LastExceptionStack;
}

// Gets a pointer to the thread-local buffer holding last exception stack trace data
ERRLIB_STACK_TRACE* ErrLib_GetStackTracePointer(){
    return (ERRLIB_STACK_TRACE*)TlsGetValue(ErrLib_tlsiStackTrace);
}

ERRLIB_API ERRLIB_STACK_TRACE __stdcall ErrLib_Except_GetStackTraceData(){
    ERRLIB_STACK_TRACE* pStack;
    pStack = ErrLib_GetStackTracePointer();
    return StackTrace_Copy(pStack);
}

//Gets filename from full path
ERRLIB_API WCHAR* __stdcall ErrLib_FileNameFromPathW(WCHAR* path){
        
	int i;WCHAR* p;
	
	size_t len = wcslen(path);
        if(len<=3)return path;

        i = len - 2;
        p = path;

        while(TRUE){
                if(path[i] == '\\' || path[i] == '/') {p = &(path[i+1]);break;}
                i--;
                if(i<0)break;
        }
        return p;
}

// *** Functions to print error messages *** 

//Gets Win32 error message for specified function name and error code
ERRLIB_API void __stdcall ErrLib_ErrorMes(LPTSTR lpszFunction,DWORD dw,WCHAR* buf) 
{ 
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;    

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Print the error message 
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    swprintf(buf,L"%s failed with error %u. %s", 
        lpszFunction, dw, lpMsgBuf);     

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

ERRLIB_API DWORD __stdcall ErrLib_GetWinapiErrorMessage(DWORD dwCode, BOOL localized, WCHAR* pOutput, int cch) 
{ 
    // Retrieve the system error message for the specified Win32 error code
    DWORD dwLangID;

    if(pOutput == NULL) return 0;
    
    StringCchCopy(pOutput, cch, L"");

    if(localized != FALSE) dwLangID = LANG_USER_DEFAULT;
    else dwLangID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    return FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwCode, dwLangID, pOutput, cch, NULL );
}

ERRLIB_API void __stdcall ErrLib_PrintError(LPTSTR lpszFunction,DWORD dw){
    WCHAR buf[1024]={0};
    ErrLib_ErrorMes(lpszFunction,dw,buf);
    wprintf(L"%s\n",buf);    
}

#ifdef __cplusplus
ERRLIB_API void __stdcall ErrLib_HResultToString(HRESULT hr,LPTSTR lpszFunction,WCHAR* buf) 
{ 
    _com_error err(hr);
    const WCHAR* errMsg = err.ErrorMessage();

    // Print the error message      
    swprintf(buf,L"%s failed with HRESULT 0x%x. %s", 
        lpszFunction, (DWORD)hr, errMsg);         
}

ERRLIB_API void __stdcall ErrLib_GetHResultMessage(HRESULT hr,WCHAR* lpOutput, int cch) 
{
    if(lpOutput == NULL) return;

    _com_error err(hr);
    const WCHAR* errMsg = err.ErrorMessage();
    StringCchCopy(lpOutput, cch, errMsg);
}
#endif

//Sets current exception callback. Specify NULL to call default callback
ERRLIB_API void __stdcall ErrLib_SetExceptionCallback(ERRLIB_EXCEPTION_CALLBACK pCallback){
        ErrLib_pCurrentExceptionCallback = pCallback;
}

//Sets current logging callback
ERRLIB_API void __stdcall ErrLib_SetLoggingCallback(ERRLIB_LOGGING_CALLBACK pCallback){
    ErrLib_pCurrentLoggingCallback = pCallback;
}

//Sets current log file path. The default is [MyDocuments]\[ExeFileName].log
ERRLIB_API void __stdcall ErrLib_SetLogFilePath(LPCWSTR path){
        if(path == NULL) return;
        StringCchCopy(ErrLib_LogFilePath,MAX_PATH,path);
}

//Sets value for the specified configuration parameter
ERRLIB_API BOOL __stdcall ErrLib_SetParameter(UINT param, UINT_PTR value){
	switch (param)
	{
		case ERRLIB_OUTPUT_LOGFILE: ErrLib_fOutputLogFile = (BOOL)value;return TRUE;
		case ERRLIB_OUTPUT_STDERR: ErrLib_fOutputStderr = (BOOL)value;return TRUE;
		case ERRLIB_OUTPUT_MBOX: ErrLib_fOutputMbox = (BOOL)value;return TRUE;	
		case ERRLIB_OUTPUT_EVENT_LOG: ErrLib_fOutputEventLog = (BOOL)value;return TRUE;
        case ERRLIB_OUTPUT_CUSTOM: ErrLib_fOutputCustom = (BOOL)value;return TRUE;

        case ERRLIB_PARAM_VISUALCPPVERSION: ErrLib_VisualCppRtVersion = (int)value;return TRUE;
        case ERRLIB_PARAM_ISDEBUGBUILD: ErrLib_fDebugBuild = (BOOL)value;return TRUE;

		default:return FALSE;
	}
}

ERRLIB_API BOOL __stdcall ErrLib_InitThread(){

    LPVOID lpvData = NULL;
    BOOL res;
    ERRLIB_STACK_TRACE stack;

		lpvData = LocalAlloc(LPTR,sizeof(ULONG_PTR) * 2);
		res = TlsSetValue(ErrLib_tlsiExArgs,lpvData);
		if(res == FALSE) fwprintf(stderr,L"TlsSetValue failed\n");

		lpvData = LocalAlloc(LPTR,sizeof(DWORD) );
		res = TlsSetValue(ErrLib_tlsiLastExceptionCode,lpvData);
		if(res == FALSE) fwprintf(stderr,L"TlsSetValue failed\n");

		lpvData = LocalAlloc(LPTR,sizeof(WCHAR) * ErrLib_MessageLen);
		res = TlsSetValue(ErrLib_tlsiLastExceptionMessage,lpvData);
		if(res == FALSE) fwprintf(stderr,L"TlsSetValue failed\n");

		lpvData = LocalAlloc(LPTR,sizeof(WCHAR) * ErrLib_StackLen);
		res = TlsSetValue(ErrLib_tlsiLastExceptionStack,lpvData);
		if(res == FALSE) fwprintf(stderr,L"TlsSetValue failed\n");

		lpvData = LocalAlloc(LPTR,sizeof(WCHAR) * ErrLib_MessageLen);
		res = TlsSetValue(ErrLib_tlsiStrBuf,lpvData);
		if(res == FALSE) fwprintf(stderr,L"TlsSetValue failed\n");

    lpvData = LocalAlloc(LPTR, sizeof(ERRLIB_STACK_TRACE));
    res = TlsSetValue(ErrLib_tlsiStackTrace,lpvData);

    if(res == FALSE) {
        fwprintf(stderr,L"TlsSetValue failed\n");
    }
    else{
        stack = StackTrace_Alloc(10);
        memcpy(lpvData, &stack, sizeof(stack));
    }

	return res;
}

ERRLIB_API void __stdcall ErrLib_FreeThread(){

	LPVOID lpvData = NULL;	

		lpvData =  TlsGetValue(ErrLib_tlsiExArgs);	
		LocalFree(lpvData);

		lpvData =  TlsGetValue(ErrLib_tlsiLastExceptionCode);	
		LocalFree(lpvData);

		lpvData =  TlsGetValue(ErrLib_tlsiLastExceptionMessage);
		LocalFree(lpvData);

		lpvData =  TlsGetValue(ErrLib_tlsiLastExceptionStack);	
		LocalFree(lpvData);

		lpvData =  TlsGetValue(ErrLib_tlsiStrBuf);
		LocalFree(lpvData);

    lpvData = TlsGetValue(ErrLib_tlsiStackTrace);

    if(lpvData != NULL){
        ErrLib_FreeStackTrace((ERRLIB_STACK_TRACE*)lpvData);
        LocalFree(lpvData);
    }
}

ERRLIB_API BOOL __stdcall ErrLib_InitTLS(){
        
	BOOL retval = TRUE;

		ErrLib_tlsiExArgs = TlsAlloc();
		if(ErrLib_tlsiExArgs == TLS_OUT_OF_INDEXES) {
			fwprintf(stderr,L"TlsAlloc failed\n");
			retval = FALSE;
		}

		ErrLib_tlsiLastExceptionCode = TlsAlloc();
		if(ErrLib_tlsiLastExceptionCode == TLS_OUT_OF_INDEXES) {
			fwprintf(stderr,L"TlsAlloc failed\n");
			retval = FALSE;
		}

		ErrLib_tlsiLastExceptionMessage = TlsAlloc();
		if(ErrLib_tlsiLastExceptionMessage == TLS_OUT_OF_INDEXES) {
			fwprintf(stderr,L"TlsAlloc failed\n");
			retval = FALSE;
		}

		ErrLib_tlsiLastExceptionStack = TlsAlloc();
		if(ErrLib_tlsiLastExceptionStack == TLS_OUT_OF_INDEXES) {
			fwprintf(stderr,L"TlsAlloc failed\n");
			retval = FALSE;
		}

		ErrLib_tlsiStrBuf = TlsAlloc();	
		if(ErrLib_tlsiStrBuf == TLS_OUT_OF_INDEXES) {
			fwprintf(stderr,L"TlsAlloc failed\n");
			retval = FALSE;
		}

    ErrLib_tlsiStackTrace = TlsAlloc();

    if(ErrLib_tlsiStackTrace == TLS_OUT_OF_INDEXES) {
        fwprintf(stderr,L"TlsAlloc failed\n");
        retval = FALSE;
    }

    return retval;        
}

//Initializes the library. Must be called before any other functionality is used.
ERRLIB_API BOOL __stdcall ErrLib_InitializeInternal(){
        BOOL res = FALSE;
		HRESULT hr=S_OK;
        PWSTR documents_path=NULL;
        WCHAR szDirectory[MAX_PATH]=L"";
        WCHAR szFileName[MAX_PATH]=L"";
        WCHAR logpath[MAX_PATH]=L"";
		LPCWSTR ShortName = NULL;
		LPVOID lpvData = NULL;

		//set global unhandled exception filter which will be called without debugger's precense
        ErrLib_pPreviousExceptionFilter = SetUnhandledExceptionFilter(&ErrLib_MyUnhandledExceptionFilter);

		//initialize thread safety objects		
		InitializeCriticalSection(&ErrLib_DbgHlpSync);
		InitializeCriticalSection(&ErrLib_LogSync);			

		//initialize debugging symbols
        res = SymInitialize( GetCurrentProcess(), NULL, TRUE ); //load symbols
        if(res == FALSE) fwprintf(stderr,L"SymInitialize failed with error 0x%x\n",GetLastError());		
		       
		//configure default log file path
		hr=SHGetKnownFolderPath(ERRLIB_REFERENCE FOLDERID_Documents,0,(HANDLE)NULL,&documents_path); //get My Documents path	
        if(SUCCEEDED(hr)){
                StringCchCopy(szDirectory,MAX_PATH,documents_path);		
                StringCchCat(szDirectory,MAX_PATH,L"\\");
                CoTaskMemFree(documents_path);
        }

        ShortName = szFileName;
		GetModuleFileName(NULL, szFileName, MAX_PATH);

        ShortName = ErrLib_FileNameFromPathW(szFileName); //get current EXE file short name
        StringCchPrintf(logpath,MAX_PATH,L"%s%s.log",szDirectory,ShortName);
        ErrLib_SetLogFilePath(logpath);	

        return res;        
}


//Creates registry data for Windows Event Log. Requires elevated priveleges. 
ERRLIB_API BOOL __stdcall ErrLib_RegisterEventSource(){
	WCHAR szFileName[ErrLib_MessageLen]=L"";
	LPCWSTR ShortName = NULL;
	WCHAR buf[ErrLib_MessageLen]=L"";
	LONG lRes;
	DWORD dwValue;
	HKEY hKey;
	HMODULE hModule = NULL;
	BOOL retval = TRUE;

	//get current EXE file short name
	if(0 == GetModuleFileName(NULL, szFileName, ErrLib_MessageLen)) {
		fwprintf(stderr,L"ErrLib_RegisterEventSource: GetModuleFileName failed with code %u\n",GetLastError());	
		return FALSE;
	}
	ShortName = ErrLib_FileNameFromPathW(szFileName);

	//set registry values
	StringCchPrintf(buf,ErrLib_MessageLen,L"SYSTEM\\CurrentControlSet\\services\\eventlog\\Application\\%s",ShortName);
	lRes=RegCreateKeyW(HKEY_LOCAL_MACHINE,buf,&hKey);
	if(lRes!=ERROR_SUCCESS){				
		return FALSE;
	}

	dwValue = 1;
	lRes=RegSetValueExW(hKey,L"CategoryCount",0,REG_DWORD,(BYTE*)&dwValue,sizeof(DWORD));
	if(lRes!=ERROR_SUCCESS){		
		retval = FALSE;
	}

	dwValue = 3;
	lRes=RegSetValueExW(hKey,L"TypesSupported",0,REG_DWORD,(BYTE*)&dwValue,sizeof(DWORD));
	if(lRes!=ERROR_SUCCESS){		
		retval = FALSE;
	}

	StringCchCopy(buf,ErrLib_MessageLen,L"");
	if(FALSE == GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,(LPCTSTR)ErrLib_RegisterEventSource,&hModule)){		
		retval = FALSE;
	}

	if(hModule != NULL) GetModuleFileName(hModule, buf, ErrLib_MessageLen);

	lRes=RegSetValueExW(hKey,L"CategoryMessageFile",0,REG_SZ,(BYTE*)buf,wcslen(buf)*2+1);
	if(lRes!=ERROR_SUCCESS){		
		retval = FALSE;
	}

	lRes=RegSetValueExW(hKey,L"EventMessageFile",0,REG_SZ,(BYTE*)buf,wcslen(buf)*2+1);
	if(lRes!=ERROR_SUCCESS){		
		retval = FALSE;
	}

	lRes=RegSetValueExW(hKey,L"ParameterMessageFile",0,REG_SZ,(BYTE*)buf,wcslen(buf)*2+1);
	if(lRes!=ERROR_SUCCESS){		
		retval = FALSE;
	}
			
	RegCloseKey(hKey);
	return retval;
}

//Deletes Windows Event Log registry data. Requires elevated priveleges. 
ERRLIB_API BOOL __stdcall ErrLib_UnregisterEventSource(){
	WCHAR szFileName[ErrLib_MessageLen]=L"";
	LPCWSTR ShortName = NULL;
	WCHAR buf[ErrLib_MessageLen]=L"";
	LONG lRes;		

	if(0 == GetModuleFileName(NULL, szFileName, ErrLib_MessageLen)) {
		fwprintf(stderr,L"ErrLib_UnregisterEventSource: GetModuleFileName failed with code %u\n",GetLastError());	
		return FALSE;
	}
	ShortName = ErrLib_FileNameFromPathW(szFileName);

	if(wcslen(ShortName) == 0) return FALSE;

	StringCchPrintf(buf,ErrLib_MessageLen,L"SYSTEM\\CurrentControlSet\\services\\eventlog\\Application\\%s",ShortName);
	lRes=RegDeleteKey(HKEY_LOCAL_MACHINE,buf);
	
	if(lRes!=ERROR_SUCCESS){			
		return FALSE;
	}
	
	return TRUE;
}

ERRLIB_STACK_TRACE StackTrace_Alloc(int capacity){
    ERRLIB_STACK_TRACE ret;
    memset(&ret,0,sizeof(ret));
    ret.data = (ERRLIB_STACK_FRAME*)LocalAlloc(LPTR, capacity * sizeof(ERRLIB_STACK_FRAME));
    ret.capacity = capacity;
    ret.isOnHeap = TRUE;
    return ret;
}

void StackTrace_Realloc(ERRLIB_STACK_TRACE* pStack, int newCapacity){
    size_t newSize = newCapacity * sizeof(ERRLIB_STACK_FRAME);
    ERRLIB_STACK_FRAME* pNewData = (ERRLIB_STACK_FRAME*)LocalAlloc(LPTR, newSize);

    if(pStack->data != NULL && pStack->isOnHeap != FALSE){
        memcpy_s(pNewData, newSize, pStack->data, pStack->count * sizeof(ERRLIB_STACK_FRAME));
        LocalFree(pStack->data);
    }

    pStack->data = pNewData;
    pStack->capacity = newCapacity;
    pStack->isOnHeap = TRUE;
}

ERRLIB_API int __stdcall ErrLib_ST_GetFramesCount(const ERRLIB_STACK_TRACE* pStack){
    if(pStack==NULL) return 0;
    return pStack->count;
}

ERRLIB_API BOOL  __stdcall ErrLib_ST_GetFrame(const ERRLIB_STACK_TRACE* pStack, int n, ERRLIB_STACK_FRAME* pOutput){
    if(pStack==NULL) return FALSE;
    if(n<0 || n>= pStack->count) return FALSE;
    memcpy_s(pOutput, sizeof(ERRLIB_STACK_FRAME), &(pStack->data[n]), sizeof(ERRLIB_STACK_FRAME));
    return TRUE;
}

ERRLIB_API uint64_t __stdcall ErrLib_ST_GetAddress(const ERRLIB_STACK_FRAME* pFrame){
    if(pFrame == NULL) return 0x0;
    return pFrame->addr;
}

ERRLIB_API uint64_t __stdcall ErrLib_ST_GetDisplacement(const ERRLIB_STACK_FRAME* pFrame){
    if(pFrame == NULL) return 0x0;
    return pFrame->displacement;
}

ERRLIB_API const WCHAR* __stdcall ErrLib_ST_GetStringProperty(const ERRLIB_STACK_FRAME* pFrame, int propId){

    if(pFrame == NULL) return NULL;

    switch(propId){
        case ERRLIB_SYMBOL_NAME:   return pFrame->symbol;
        case ERRLIB_SYMBOL_MODULE: return pFrame->module;
        case ERRLIB_SYMBOL_SOURCE: return pFrame->src_file;
        default: return NULL;
    }
}

ERRLIB_API DWORD __stdcall ErrLib_ST_GetSymLine(const ERRLIB_STACK_FRAME* pFrame){
    if(pFrame == NULL) return 0;
    return pFrame->src_line;
}

ERRLIB_API void __stdcall ErrLib_FreeStackTrace(ERRLIB_STACK_TRACE* pStack){
    
    if(pStack->data != NULL && pStack->isOnHeap != FALSE){
        LocalFree(pStack->data);
    }

    pStack->data = NULL;
    pStack->capacity = 0;
    pStack->count = 0;
}

void StackTrace_AddFrame(ERRLIB_STACK_TRACE* pStack,const ERRLIB_STACK_FRAME* pFrame){

    if(pStack->count + 1 > pStack->capacity) StackTrace_Realloc(pStack, pStack->capacity * 2);

    pStack->data[pStack->count] = *pFrame;
    pStack->count++;
}

ERRLIB_STACK_TRACE StackTrace_Copy(const ERRLIB_STACK_TRACE* pInput){
    ERRLIB_STACK_TRACE output;
    ERRLIB_STACK_FRAME* pNewData = NULL;
    int newCapacity = pInput->count;

    if(newCapacity<10) newCapacity=10;

    pNewData = (ERRLIB_STACK_FRAME*)LocalAlloc(LPTR, newCapacity * sizeof(ERRLIB_STACK_FRAME));

    if(pInput->data != NULL && pInput->count>0){
        memcpy_s(pNewData, newCapacity * sizeof(ERRLIB_STACK_FRAME), pInput->data, pInput->count * sizeof(ERRLIB_STACK_FRAME));
    }

    output.capacity = newCapacity;
    output.count = pInput->count;
    output.data = pNewData;
    output.isOnHeap = TRUE;
    return output;
}

void ErrLib_GetStackTraceImpl(CONTEXT* ctx, ERRLIB_STACK_TRACE* pOutput) 
{
    BOOL    result  = FALSE;
    HANDLE  process = NULL;
    HANDLE  thread  = NULL;
    HMODULE hModule = NULL;

    STACKFRAME64        stack;
    ULONG               frame=0;    
    DWORD64             displacement=0;

    DWORD disp = 0;
    IMAGEHLP_LINEW64 *line = NULL;

    char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)]={0};    
    WCHAR module[ErrLib_MaxNameLen]={0};
    ERRLIB_STACK_FRAME stackFrame;

    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;
    BOOL symbolFound = FALSE;
    BOOL lineinfoFound = FALSE;

    // Context record could be modified by StackWalk64, which causes crashes on x64 when the context comes from the
    // SEH exception information. So we create a copy here to prevent it.
    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalk64
    // https://github.com/MSDN-WhiteKnight/ErrLib/issues/2
    CONTEXT ctxCopy;
    memcpy(&ctxCopy, ctx, sizeof(CONTEXT));    
    
    pOutput->count = 0;
    memset( &stack, 0, sizeof( STACKFRAME64 ) );

    process                = GetCurrentProcess();
    thread                 = GetCurrentThread();
    displacement           = 0;
#if !defined(_M_AMD64)
    stack.AddrPC.Offset    = (*ctx).Eip;
    stack.AddrPC.Mode      = AddrModeFlat;
    stack.AddrStack.Offset = (*ctx).Esp;
    stack.AddrStack.Mode   = AddrModeFlat;
    stack.AddrFrame.Offset = (*ctx).Ebp;
    stack.AddrFrame.Mode   = AddrModeFlat;
#endif

	EnterCriticalSection(&ErrLib_DbgHlpSync);
	__try{

    for( frame = 0; ; frame++ )
    {
        //get next call from stack
        result = StackWalk64
        (
#if defined(_M_AMD64)
            IMAGE_FILE_MACHINE_AMD64
#else
            IMAGE_FILE_MACHINE_I386
#endif
            ,
            process,
            thread,
            &stack,
            &ctxCopy,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL
        );

        if( !result ) break;

        memset(&stackFrame, 0, sizeof(stackFrame));

        //get symbol name for address
        ZeroMemory(buffer,sizeof(buffer));
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
        pSymbol->MaxNameLen = MAX_SYM_NAME;
        symbolFound = SymFromAddrW(process, (ULONG64)stack.AddrPC.Offset, &displacement, pSymbol);

        if(symbolFound == FALSE){ //name not available, output address instead
            StringCchPrintf(pSymbol->Name,MAX_SYM_NAME,L"0x%llx",(DWORD64)stack.AddrPC.Offset);
        }

        stackFrame.addr = stack.AddrPC.Offset;
        stackFrame.displacement = displacement;
        StringCchCopy(stackFrame.symbol, MAX_SYM_NAME, pSymbol->Name);

        line = (IMAGEHLP_LINEW64 *)malloc(sizeof(IMAGEHLP_LINEW64));
                ZeroMemory(line,sizeof(IMAGEHLP_LINEW64));
        line->SizeOfStruct = sizeof(IMAGEHLP_LINEW64);  

        //get module name
        hModule = NULL;		
        wcscpy(module,L"");   
        
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                (LPCTSTR)(stack.AddrPC.Offset), &hModule);

        if(hModule != NULL){
            GetModuleFileName(hModule,module,ErrLib_MaxNameLen);
            StringCchCopy(stackFrame.module, MAX_PATH, module);
        }

        //try to get line
        lineinfoFound = FALSE;

        if (symbolFound != FALSE) {
            // Only try to find line info when symbol is found - fixes crash when Win7 DbgHelp reads PDB symbols 
            // built with /DEBUG:FASTLINK option
            // (https://github.com/MSDN-WhiteKnight/ErrLib/issues/2)
            lineinfoFound = SymGetLineFromAddrW64(process, stack.AddrPC.Offset, &disp, line);
        }

        if (lineinfoFound != FALSE)
        {
            StringCchCopy(stackFrame.src_file, MAX_PATH,line->FileName);
            stackFrame.src_line = line->LineNumber;
        }
        
        StackTrace_AddFrame(pOutput, &stackFrame);
        free(line);
        line = NULL;
		if(frame > 9999)break;
    }//end for

	}__finally{
	   LeaveCriticalSection(&ErrLib_DbgHlpSync);
	}
}

ERRLIB_API ERRLIB_STACK_TRACE __stdcall ErrLib_GetStackTrace(CONTEXT* ctx){
    ERRLIB_STACK_TRACE ret = StackTrace_Alloc(50);
    ErrLib_GetStackTraceImpl(ctx, &ret);
    return ret;
}

//Prints stack trace based on context record
ERRLIB_API void __stdcall ErrLib_PrintStack( CONTEXT* ctx , WCHAR* dest, size_t cch) 
{
    BOOL    result = FALSE;
    HANDLE  process= NULL;
    HANDLE  thread= NULL;
    HMODULE hModule= NULL;

    STACKFRAME64        stack;
    ULONG               frame=0;    
    DWORD64             displacement=0;

    DWORD disp = 0;
    IMAGEHLP_LINEW64 *line = NULL;

    char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)]={0};
    WCHAR name[ ErrLib_MaxNameLen ]={0};
    WCHAR module[ErrLib_MaxNameLen]={0};
    WCHAR* module_short = NULL;
    WCHAR strbuf[MAX_SYM_NAME*3]={0};

    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;
    BOOL symbolFound = FALSE;
    BOOL lineinfoFound = FALSE;

    // Context record could be modified by StackWalk64, which causes crashes on x64 when the context comes from the
    // SEH exception information. So we create a copy here to prevent it.
    // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-stackwalk64
    // https://github.com/MSDN-WhiteKnight/ErrLib/issues/2
    CONTEXT ctxCopy;
    memcpy(&ctxCopy, ctx, sizeof(CONTEXT));    

    StringCchCopy(dest,cch,L"");
    memset( &stack, 0, sizeof( STACKFRAME64 ) );

    process                = GetCurrentProcess();
    thread                 = GetCurrentThread();
    displacement           = 0;
#if !defined(_M_AMD64)
    stack.AddrPC.Offset    = (*ctx).Eip;
    stack.AddrPC.Mode      = AddrModeFlat;
    stack.AddrStack.Offset = (*ctx).Esp;
    stack.AddrStack.Mode   = AddrModeFlat;
    stack.AddrFrame.Offset = (*ctx).Ebp;
    stack.AddrFrame.Mode   = AddrModeFlat;
#endif

	EnterCriticalSection(&ErrLib_DbgHlpSync);
	__try{

    for( frame = 0; ; frame++ )
    {
        //get next call from stack
        result = StackWalk64
        (
#if defined(_M_AMD64)
            IMAGE_FILE_MACHINE_AMD64
#else
            IMAGE_FILE_MACHINE_I386
#endif
            ,
            process,
            thread,
            &stack,
            &ctxCopy,
            NULL,
            SymFunctionTableAccess64,
            SymGetModuleBase64,
            NULL
        );

        if( !result ) break;        

        //get symbol name for address
        ZeroMemory(buffer,sizeof(buffer));
        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
        pSymbol->MaxNameLen = MAX_SYM_NAME;
        symbolFound = SymFromAddrW(process, ( ULONG64 )stack.AddrPC.Offset, &displacement, pSymbol);

        if(symbolFound == FALSE){ //name not available, output address instead
            StringCchPrintf(pSymbol->Name,MAX_SYM_NAME,L"0x%llx",(DWORD64)stack.AddrPC.Offset);
        }

        line = (IMAGEHLP_LINEW64 *)malloc(sizeof(IMAGEHLP_LINEW64));
                ZeroMemory(line,sizeof(IMAGEHLP_LINEW64));
        line->SizeOfStruct = sizeof(IMAGEHLP_LINEW64);  

        //get module name
        hModule = NULL;		
        wcscpy(module,L"");   
        module_short=&(module[0]);

        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                (LPCTSTR)(stack.AddrPC.Offset), &hModule);

        if(hModule != NULL){
                GetModuleFileName(hModule,module,ErrLib_MaxNameLen); 
                module_short = ErrLib_FileNameFromPathW(module);
        }

        //try to get line
        lineinfoFound = FALSE;

        if (symbolFound != FALSE) {
            // Only try to find line info when symbol is found - fixes crash when Win7 DbgHelp reads PDB symbols 
            // built with /DEBUG:FASTLINK option
            // (https://github.com/MSDN-WhiteKnight/ErrLib/issues/2)
            lineinfoFound = SymGetLineFromAddrW64(process, stack.AddrPC.Offset, &disp, line);
        }

        if (lineinfoFound != FALSE)
        {
            StringCchPrintf(strbuf,sizeof(strbuf)/sizeof(WCHAR),
                            L"  in %s!%s + 0x%02llx (%s; line: %lu;)\n", module_short, pSymbol->Name, 
                            displacement, line->FileName, line->LineNumber);
            StringCchCat(dest,cch,strbuf);
        }
        else
        { 
            //failed to get line, output address instead
            StringCchPrintf(strbuf,sizeof(strbuf)/sizeof(WCHAR),
                            L"  in %s!%s (%s; address: 0x%llx)\n", module_short,pSymbol->Name, module,
                            (DWORD64)(stack.AddrPC.Offset - pSymbol->ModBase));
            StringCchCat(dest,cch,strbuf);
        }

        free(line);
        line = NULL;
		if(frame > 9999)break;
    }//end for

	}__finally{
	   LeaveCriticalSection(&ErrLib_DbgHlpSync);
	}
}

//Prints error message for specified exception
ERRLIB_API void __stdcall ErrLib_GetExceptionMessage(struct _EXCEPTION_POINTERS *ExceptionInfo, LPWSTR dest, size_t cch){
        WCHAR buf[1024]={0};	

        // https://msdn.microsoft.com/en-us/library/cc704588.aspx?f=255&MSPPError=-2147217396
        //https://msdn.microsoft.com/ru-ru/library/windows/desktop/aa363082%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396

        switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
        {
                case STATUS_CONTROL_C_EXIT: StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"STATUS_CONTROL_C_EXIT");break;

                case STATUS_ACCESS_VIOLATION : 
                        switch (ExceptionInfo->ExceptionRecord->ExceptionInformation[0])
                        {
                        case 0: 
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction referenced memory at 0x%08llx. The memory could not be read",
                        (DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
                                break;
                        case 1:
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction referenced memory at 0x%08llx. The memory could not be written",
                        (DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
                                break;
                        default:
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction referenced memory address for which it does not have the appropriate access"
                        );
                                break;
                        }
                        break;		

                case STATUS_IN_PAGE_ERROR: 
                        switch (ExceptionInfo->ExceptionRecord->ExceptionInformation[0])
                        {
                        case 0: 
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction tried to read memory at 0x%08llx, but the system was unable to load page because of error 0x%08llx",
                        (DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[1],(DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[2]);
                                break;
                        case 1:
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction tried to write memory at 0x%08llx, but the system was unable to load page because of error 0x%08llx",
                        (DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[1],(DWORD64)ExceptionInfo->ExceptionRecord->ExceptionInformation[2]);
                                break;
                        default:
                                StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"The instruction tried to access a page that was not present, and the system was unable to load the page"
                        );
                                break;
                        }			
                        break;				

                case 0xe06d7363 : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"C++ exception was not handled in user code");break;
                case STATUS_ARRAY_BOUNDS_EXCEEDED : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Array bounds exceeded");break;
                case STATUS_NO_MEMORY : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"Not enough virtual memory or paging file quota is available to complete the specified operation");
                        break;
                case STATUS_STACK_OVERFLOW : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"The thread used up all of its available stack space");
                        break;
                case STATUS_STACK_BUFFER_OVERRUN : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),L"The system detected an overrun of a stack-based buffer in this application");
                        break;
                case STATUS_ILLEGAL_INSTRUCTION : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"An attempt was made to execute an illegal instruction");
                        break;		
                case STATUS_INVALID_CRUNTIME_PARAMETER : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"An invalid parameter was passed to a C runtime function.");
                        break;		

                case STATUS_FLOAT_DENORMAL_OPERAND : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point denormal operand");break;
                case STATUS_FLOAT_DIVIDE_BY_ZERO  : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point division by zero");break;
                case STATUS_FLOAT_INEXACT_RESULT   : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point inexact result");break;
                case STATUS_FLOAT_INVALID_OPERATION   : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point invalid operation");break;
                case STATUS_FLOAT_OVERFLOW        : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point overflow");break;
                case STATUS_FLOAT_STACK_CHECK     : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L" Floating-point stack check");break;
                case STATUS_FLOAT_UNDERFLOW      : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Floating-point underflow");break;
                case STATUS_FLOAT_MULTIPLE_FAULTS     : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Multiple floating-point faults");break;
                case STATUS_FLOAT_MULTIPLE_TRAPS      : StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Multiple floating-point traps");break;

                case STATUS_INTEGER_DIVIDE_BY_ZERO : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Integer division by zero");break;
                case STATUS_INTEGER_OVERFLOW : 
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR), L"Integer operation resulted in overflow");break;

                case ERRLIB_WIN32_EXCEPTION: 			
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"%s",(LPCWSTR)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
                        break;	

                case ERRLIB_APP_EXCEPTION: 			
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"%s",(LPCWSTR)ExceptionInfo->ExceptionRecord->ExceptionInformation[0]);
                        break;	

#ifdef __cplusplus
                case ERRLIB_COM_EXCEPTION: 			
                        StringCchPrintf(buf,sizeof(buf)/sizeof(WCHAR),
                        L"%s",(LPCWSTR)ExceptionInfo->ExceptionRecord->ExceptionInformation[1]);
                        break;
#endif
        }

    StringCchPrintf(dest,cch,L"%s",buf);	
}

//Outputs exception information into configured log targets
ERRLIB_API void __stdcall ErrLib_LogExceptionInfo(DWORD dwExcCode,LPCWSTR lpwsMessage,LPCWSTR lpwsStackTrace, BOOL visible){

	SYSTEMTIME st={0};
	FILE *fp = NULL;
	WCHAR buf[ErrLib_StackLen]={0};
	WCHAR szFileName[MAX_PATH]=L"";
	LPCWSTR ShortName = NULL;	
	HANDLE hEventLog = NULL;
	LPWSTR pInsertStrings[1] = {NULL};

	EnterCriticalSection(&ErrLib_LogSync);

	__try{

	//put information into the log file
	if(ErrLib_fOutputLogFile != FALSE){
		fp = _wfopen(ErrLib_LogFilePath, L"at+, ccs=UTF-8");
		if(fp != NULL){
              
              GetLocalTime(&st);
              fwprintf(fp,L"%4d.%02d.%02d %2d:%02d - Exception 0x%x: ",
				  (int)st.wYear,(int)st.wMonth,(int)st.wDay,(int)st.wHour,(int)st.wMinute,dwExcCode);
              fputws(lpwsMessage,fp);   
			  fputwc(L'\n',fp);

              fputws(lpwsStackTrace,fp);
              fputwc(L'\n',fp);
              fclose(fp);
		}
	}

	//put information into the Windows Event Log
	if(ErrLib_fOutputEventLog != FALSE){
		GetModuleFileName(NULL, szFileName, MAX_PATH);
		ShortName = ErrLib_FileNameFromPathW(szFileName);

		hEventLog = RegisterEventSource(NULL, ShortName);
		if (NULL == hEventLog)
		{
			fwprintf(stderr,L"RegisterEventSource failed with 0x%x.\n", GetLastError());			
		}else{
			// This event uses insert strings.
			StringCchPrintf(buf,ErrLib_StackLen,L"Exception 0x%x: %s\n%s",dwExcCode,lpwsMessage,lpwsStackTrace);
			pInsertStrings[0] = buf;			
			if (!ReportEvent(hEventLog, EVENTLOG_ERROR_TYPE, ERRLIB_CATEGORY, MSG_ERROR, NULL, 1, 0, (LPCWSTR*)pInsertStrings, NULL))
			{
				fwprintf(stderr,L"ReportEvent failed with 0x%x.\n", GetLastError());				
			}
		}
	}

	//put information into stderr stream
	if(visible != FALSE && ErrLib_fOutputStderr!=FALSE){
		fwprintf(stderr,L"Exception 0x%x: ",dwExcCode);
		fputws(lpwsMessage,stderr);
		fputwc(L'\n',stderr);
		fputws(lpwsStackTrace,stderr);
	}

	//display information in message box
	if(visible != FALSE && ErrLib_fOutputMbox!=FALSE){
		StringCchPrintf(buf,ErrLib_StackLen,L"Exception 0x%x: %s",dwExcCode,lpwsMessage);
		MessageBox(NULL,buf,NULL,MB_SYSTEMMODAL | MB_OK | MB_ICONERROR);
	}

    //invoke custom target
    if(ErrLib_fOutputCustom != FALSE && ErrLib_pCurrentLoggingCallback != NULL){
        StringCchPrintf(buf,ErrLib_StackLen,L"Exception 0x%x: %s\r\n%s",dwExcCode,lpwsMessage,lpwsStackTrace);
        ErrLib_pCurrentLoggingCallback(buf, NULL);
    }

	}__finally{
	  LeaveCriticalSection(&ErrLib_LogSync);
	}
}

//Outputs arbitrary string into configured log targets (type can be: MSG_ERROR, MSG_WARNING or MSG_INFORMATION)
ERRLIB_API void __stdcall ErrLib_LogMessage(LPCWSTR lpwsMessage, BOOL visible, DWORD type, BOOL bIncludeStack ){

	SYSTEMTIME st={0};
	FILE *fp = NULL;
	WCHAR lpwsStackTrace[ErrLib_StackLen]={0};
	WCHAR buf[ErrLib_StackLen]={0};
	WCHAR szFileName[MAX_PATH]=L"";
	LPCWSTR ShortName = NULL;	
	LPCWSTR caption = NULL;
	HANDLE hEventLog = NULL;
	LPWSTR pInsertStrings[1] = {NULL};
	CONTEXT ctx = {0};
	DWORD eventlog_type;
	DWORD mbox_type;

	if(bIncludeStack != FALSE){
		RtlCaptureContext(&ctx);
		ErrLib_PrintStack(&ctx,lpwsStackTrace,ErrLib_StackLen);
	}

	EnterCriticalSection(&ErrLib_LogSync);

	__try{

	//put information into the log file
	if(ErrLib_fOutputLogFile != FALSE){
		fp = _wfopen(ErrLib_LogFilePath, L"at+, ccs=UTF-8");
		if(fp != NULL){
              
              GetLocalTime(&st);
              fwprintf(fp,L"%4d.%02d.%02d %2d:%02d: ",
				  (int)st.wYear,(int)st.wMonth,(int)st.wDay,(int)st.wHour,(int)st.wMinute);
              fputws(lpwsMessage,fp);   
			  fputwc(L'\n',fp);

              fputws(lpwsStackTrace,fp);
              fputwc(L'\n',fp);
              fclose(fp);
		}
	}

	//put information into the event log
	if(ErrLib_fOutputEventLog != FALSE){

		switch(type){
			case MSG_ERROR: eventlog_type = EVENTLOG_ERROR_TYPE;break;
			case MSG_WARNING: eventlog_type =	EVENTLOG_WARNING_TYPE;break;
			case MSG_INFORMATION: eventlog_type = EVENTLOG_INFORMATION_TYPE ;break;
			default: eventlog_type = EVENTLOG_INFORMATION_TYPE ;break;
		}

		GetModuleFileName(NULL, szFileName, MAX_PATH);
		ShortName = ErrLib_FileNameFromPathW(szFileName);

		hEventLog = RegisterEventSource(NULL, ShortName);
		if (NULL == hEventLog)
		{
			fwprintf(stderr,L"RegisterEventSource failed with 0x%x.\n", GetLastError());			
		}else{
			// This event uses insert strings.
			StringCchPrintf(buf,ErrLib_StackLen,L"%s\n%s",lpwsMessage,lpwsStackTrace);
			pInsertStrings[0] = buf;			
			if (!ReportEvent(hEventLog, eventlog_type, ERRLIB_CATEGORY, type, NULL, 1, 0, (LPCWSTR*)pInsertStrings, NULL))
			{
				fwprintf(stderr,L"ReportEvent failed with 0x%x.\n", GetLastError());				
			}
		}
	}

	//put information into stderr stream
	if(visible != FALSE && ErrLib_fOutputStderr!=FALSE){
		
		fputws(lpwsMessage,stderr);
		fputwc(L'\n',stderr);
		fputws(lpwsStackTrace,stderr);
	}

	//display information as message box
	if(visible != FALSE && ErrLib_fOutputMbox!=FALSE){
		
		switch(type){
			case MSG_ERROR: mbox_type = MB_ICONERROR;break;
			case MSG_WARNING: mbox_type = MB_ICONWARNING; caption = L"Warning";break;
			case MSG_INFORMATION: mbox_type = MB_ICONINFORMATION;caption = L"Information";break;
			default: mbox_type = 0;break;
		}

		MessageBox(NULL,lpwsMessage,caption,MB_SYSTEMMODAL | MB_OK | mbox_type);
	}

    //invoke custom target
    if(ErrLib_fOutputCustom != FALSE && ErrLib_pCurrentLoggingCallback != NULL){
        ZeroMemory(buf, sizeof(buf));
        StringCchCat(buf, ErrLib_StackLen, lpwsMessage);

        if(bIncludeStack){
            StringCchCat(buf, ErrLib_StackLen, L"\r\n");
            StringCchCat(buf, ErrLib_StackLen, lpwsStackTrace);
        }

        ErrLib_pCurrentLoggingCallback(buf, NULL);
    }

	}__finally{
	  LeaveCriticalSection(&ErrLib_LogSync);
	}
}

// *** Getters ***

//Get exception code for current exception in ERRLIB_CATCH/ERRLIB_CATCH_ALL block
ERRLIB_API DWORD __stdcall ErrLib_Except_GetCode(){
	return *((DWORD*)ErrLib_LastExceptionCode_GetPointer());
}

// *** Catch functions ***

ERRLIB_API LONG __stdcall ErrLib_CatchCode( struct _EXCEPTION_POINTERS * ex, DWORD FilteredCode){

    if(ex->ExceptionRecord->ExceptionCode == FilteredCode){
        *((DWORD*)ErrLib_LastExceptionCode_GetPointer()) = ex->ExceptionRecord->ExceptionCode;
        ErrLib_GetExceptionMessage(ex,ErrLib_Except_GetMessage(),ErrLib_MessageLen);

        if (!IsStackTraceDisabled()) {
            ErrLib_PrintStack(ex->ContextRecord, ErrLib_Except_GetStackTrace(), ErrLib_StackLen);

            ERRLIB_STACK_TRACE* pStack = ErrLib_GetStackTracePointer();
            pStack->count = 0;
            ErrLib_GetStackTraceImpl(ex->ContextRecord, pStack);
        }

        return EXCEPTION_EXECUTE_HANDLER;
    }
    else return EXCEPTION_CONTINUE_SEARCH;
}

ERRLIB_API LONG __stdcall ErrLib_CatchAll( struct _EXCEPTION_POINTERS * ex){

    *((DWORD*)ErrLib_LastExceptionCode_GetPointer()) = ex->ExceptionRecord->ExceptionCode;
    ErrLib_GetExceptionMessage(ex,ErrLib_Except_GetMessage(),ErrLib_MessageLen);

    if (!IsStackTraceDisabled()) {
        ErrLib_PrintStack(ex->ContextRecord, ErrLib_Except_GetStackTrace(), ErrLib_StackLen);
        
        ERRLIB_STACK_TRACE* pStack = ErrLib_GetStackTracePointer();
        pStack->count = 0;
        ErrLib_GetStackTraceImpl(ex->ContextRecord, pStack);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

#ifdef __cplusplus
} //extern "C" 
#endif

//called on unhandled exception
LONG WINAPI ErrLib_MyUnhandledExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo){
		
        WCHAR mes[ErrLib_MessageLen]={0};
        WCHAR buf[ErrLib_StackLen]={0};
        LONG result = EXCEPTION_CONTINUE_SEARCH;
			
        if(ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_CONTROL_C_EXIT) return EXCEPTION_CONTINUE_SEARCH;

        ErrLib_GetExceptionMessage(ExceptionInfo,mes,sizeof(mes)/sizeof(WCHAR));
        ErrLib_PrintStack(ExceptionInfo->ContextRecord,buf,sizeof(buf)/sizeof(WCHAR));

        //call user callback, if one exists, otherwise call default callback
        if(ErrLib_pCurrentExceptionCallback != NULL)
        {			
              result = ErrLib_pCurrentExceptionCallback(ExceptionInfo,mes,buf);	
        }
        else
        {					
              result = ErrLib_DefaultExceptionCallback(ExceptionInfo,mes,buf);
        }	

		
        //call previous exception filter, if there's one
        if(ErrLib_pPreviousExceptionFilter != NULL){
                result = ErrLib_pPreviousExceptionFilter(ExceptionInfo);
        }

    return result; 
    /*system("PAUSE");
    exit(1);	*/		
}

LONG WINAPI ErrLib_DefaultExceptionCallback ( struct _EXCEPTION_POINTERS * ex,LPCWSTR lpwsMessage,LPCWSTR lpwsStackTrace){
		
    ErrLib_LogExceptionInfo(ex->ExceptionRecord->ExceptionCode,lpwsMessage,lpwsStackTrace, TRUE);	
    return EXCEPTION_CONTINUE_SEARCH; //crash program
}

