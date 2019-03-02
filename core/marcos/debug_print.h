#pragma once

#include <iostream>
//#define DebugPrint(...) 
//#define DebugPrintEx(...) 



#define PKN_ENABLE_DEBUG_PRINT 0


#ifndef PKN_ENABLE_DEBUG_PRINT
#ifdef _DEBUG
#define PKN_ENABLE_DEBUG_PRINT 1
#endif
#endif

#if PKN_ENABLE_DEBUG_PRINT

#ifdef _KERNEL_MODE
#define DebugPrint( ...) DbgPrint(__VA_ARGS__)
#define DebugPrintEx( ...) DbgPrintEx(__VA_ARGS__)
#else


#define DebugPrint( ...) fprintf(stderr, __VA_ARGS__)
#define DebugPrintEx(level, ...) fprintf(stderr, __VA_ARGS__)

#endif


#else

#define DebugPrint( ...) void()
#define DebugPrintEx(level, ...) void()


#endif