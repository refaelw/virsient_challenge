#ifndef _CROSSPLATFORM_H_
#define _CROSSPLATFORM_H_

#include <stdint.h>

// Because windows sucks at exporting DLLs
#ifdef _WIN32
#ifdef LIBRARY_EXPORTS
#define LIBRARY_API __declspec(dllexport)
#else
#define LIBRARY_API __declspec(dllimport)
#endif
#elif
#define LIBRARY_API
#endif

#define EXTERN_DLL_EXPORT __declspec(dllexport)

#endif