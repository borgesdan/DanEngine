#ifndef DAN_MACROS_API_HPP
#define DAN_MACROS_API_HPP

//Macros para exportação
#ifdef _WIN32
#ifdef DAN_STATIC
// Se for estático, não usamos import/export
#define DAN_API
#else
// Se for DLL, verificamos se estamos construindo ou consumindo a DLL
#ifdef DAN_EXPORT_DLL
#define DAN_API __declspec(dllexport)
#else
#define DAN_API __declspec(dllimport)
#endif
#endif
#else
// Para Linux/Mac, deixamos vazio ou usamos __attribute__((visibility("default"))))
#define DAN_API
#endif

#endif