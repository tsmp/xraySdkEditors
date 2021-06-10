#pragma once

typedef signed   char int8;
typedef unsigned char uint8;
typedef signed   short int16;
typedef unsigned short uint16;
typedef signed   int int32;
typedef unsigned int uint32;
typedef char bchar8;
typedef wchar_t bchar16;


#ifdef UNICODE
typedef  bchar16 bchar;
#else
typedef  bchar8 bchar;
#endif

#if CURRENT_COMPILER == COMPILER_MSVC
typedef signed   __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef signed   long long int64;
typedef unsigned  long long uint64;
#endif

#if CURRENT_PROCCESOR == PROCCESOR_
typedef uint32 bsize;
typedef int32 bint;
#else
typedef uint64 bsize;
typedef int64 bint;
#endif


#if CURRENT_PLATFORM == PLATFORM_WINDOWS
#include <Windows.h>
#endif
#if CURRENT_COMPILER == COMPILER_MSVC
#define BEAR_PACKED
#else
#define BEAR_PACKED  __attribute__((__packed__))
#endif
#ifdef BEAR_STATIC_LIBRARIES
#define BEARDLL_EXPORT 
#define BEARDLL_IMPORT 
#else
#define BEARDLL_EXPORT __declspec(dllexport)
#define BEARDLL_IMPORT __declspec(dllimport)
#endif

#if CURRENT_COMPILER == COMPILER_MSVC
#define BEAR_ALIGNED(x) __declspec(align(x))
#else
#define BEAR_ALIGNED(x) __attribute__ ((aligned(x)))
#endif

#if CURRENT_COMPILER == COMPILER_MSVC
#pragma warning(disable:4200)
#pragma warning(disable:4251)
#pragma warning(disable:4201)
#pragma warning(disable:4100)
#endif