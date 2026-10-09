#pragma once

#if defined(_WIN32)
#define DAN_PLATFORM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define DAN_PLATFORM_MACOS 1
#elif defined(__ANDROID__)
#define DAN_PLATFORM_ANDROID 1
#elif defined(__linux__)
#define DAN_PLATFORM_LINUX 1
#elif defined(__unix__)
#define DAN_PLATFORM_UNIX 1
#else
#error "Sistema operacional não suportado"
#endif
#ifndef DAN_PLATFORM_WINDOWS
#define DAN_PLATFORM_WINDOWS 0
#endif
#ifndef DAN_PLATFORM_MACOS
#define DAN_PLATFORM_MACOS 0
#endif
#ifndef DAN_PLATFORM_LINUX
#define DAN_PLATFORM_LINUX 0
#endif
#ifndef DAN_PLATFORM_ANDROID
#define DAN_PLATFORM_ANDROID 0
#endif
#ifndef DAN_PLATFORM_UNIX
#define DAN_PLATFORM_UNIX 0
#endif

