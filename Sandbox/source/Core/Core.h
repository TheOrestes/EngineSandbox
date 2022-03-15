#pragma once

#ifdef BUILD_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif