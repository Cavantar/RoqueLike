#pragma once

#ifdef JPB_DLL_BUILD
#define DllExport __declspec( dllexport )
#else
#define DllExport 
#endif


