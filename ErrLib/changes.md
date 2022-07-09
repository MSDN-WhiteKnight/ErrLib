# ErrLib changes

v1.1 (09.07.2022)

- Add support for custom logging target (ErrLib_SetLoggingCallback)
- Add C++ exception API (ErrLib_CPP.h)
- Add high-level stack trace API (ErrLib_Except_GetStackTraceData)
- Add NuGet package
- Add API docs using Doxygen
- Fix crash in ErrLib_PrintStack on x64
- Fix ErrLib_PrintStack crash on Win7 when reading PDB symbols built with /DEBUG:FASTLINK option
