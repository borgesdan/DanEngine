#ifndef DAN_COMMON_COREASSERT_HPP
#define DAN_COMMON_COREASSERT_HPP

#include "../Log.hpp"

#ifdef _DEBUG
#if defined(_MSC_VER)
#define DAN_CORE_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define DAN_CORE_DEBUG_BREAK() __builtin_trap()
#else
#include <csignal>
#define DAN_CORE_DEBUG_BREAK() raise(SIGTRAP)
#endif

// Macro que recebe a condição e os argumentos de mensagem formatados
#define DAN_CORE_ASSERT(condition, ...) \
        do { \
            if (!(condition)) { \
                DAN_ENGINE_CRITICAL("Falha de Asserção: {0}", __VA_ARGS__); \
                DAN_CORE_DEBUG_BREAK(); \
            } \
        } while(false)
#else
#define DAN_CORE_DEBUG_BREAK()
#define DAN_CORE_ASSERT(condition, ...) do { } while(false)
#endif

#endif //DAN_COMMON_COREASSERT_HPP