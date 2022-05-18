//Project: ErrLib
//Author: MSDN.WhiteKnight (https://github.com/MSDN-WhiteKnight)
#ifndef VCDEFS_H
#define VCDEFS_H

//https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-160
const int VISUAL_STUDIO_V2015 = 1900;
const int VISUAL_STUDIO_V2019 = 1920;

#if defined(_MSC_VER)
const int VisualCppVersion = _MSC_VER;
#else
const int VisualCppVersion = 0;
#endif

#if defined(_M_AMD64)
const BOOL IsCPUx64 = TRUE;
#else
const BOOL IsCPUx64 = FALSE;
#endif

#endif
