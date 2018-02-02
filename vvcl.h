#pragma once

#if defined(_WIN32)
#if defined(__VCL_BUILD__)
#define VCL_API	__declspec(dllexport)
#else
#define VCL_API	__declspec(dllimport)
#endif
#else
#define VCL_API
#endif

#if !defined(EXTERN_C)
#if defined(__cplusplus)
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif

#if defined(_WIN32)
#if !defined(CDECL)
#define CDECL __cdecl
#else
#define CDECL
#endif
#else
#define CDECL
#endif

#define MAX_NAME 200

#define RECORDER_OK			'\0'
#define RECORDER_WARNING	'w'
#define RECORDER_ERROR		'e'

EXTERN_C char VCL_API CDECL getAvailableEncoderName(int encoderIndex, char encoderName[MAX_NAME]);
EXTERN_C char VCL_API CDECL recorderInitialize(int resolutionX, int resolutionY, const char* fileAndPath, int framerate, int encoderIndex);
EXTERN_C char VCL_API CDECL recorderAddFrame(const unsigned char* buffer); // 3*resolutionX*resolutionY bytes, where each pixel has RGB values
EXTERN_C char VCL_API CDECL recorderEnd(void);
