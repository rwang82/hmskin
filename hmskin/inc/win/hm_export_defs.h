#ifndef __HM_EXPORTSDEF_H__
#define __HM_EXPORTSDEF_H__

// The following block is the standard way of creating macros which make
// exporting from a DLL simpler. all files within this DLL are compiled with
// the HMLIB_EXPORTS symbol defined on the command line. this symbol must
// NOT be defined on any other project that uses this DLL, so that source
// files including this header see HMLIB_API functions as imported from this
// DLL, while this DLL sees the macro-decorated symbols as exported.
#ifdef HMLIB_EXPORTS
#define HMLIB_API __declspec(dllexport)
#else
#define HMLIB_API __declspec(dllimport)
#endif

#define EXTERN_C_START extern "C" {
#define EXTERN_C_END }


#endif //__HM_EXPORTSDEF_H__