#pragma once

#include <limits>
#include <stdint.h>
#include <thread>

#include "core/forwarding.h"

#ifdef _MSC_VER
#   define ARK_API_EXPORT_DECLARATION __declspec(dllexport)
#   define ARK_API_IMPORT_DECLARATION __declspec(dllimport)
#elif __GNUC__
#	define ARK_API_EXPORT_DECLARATION __attribute__ ((visibility ("default")))
#	define ARK_API_IMPORT_DECLARATION __attribute__ ((visibility ("default")))
#else
#   define ARK_API_EXPORT_DECLARATION
#   define ARK_API_IMPORT_DECLARATION
#endif

#if defined(__GNUC__) || defined(__clang__)
#define ARK_DEPRECATED(message) __attribute__((deprecated(message)))
#elif defined(_MSC_VER)
#define ARK_DEPRECATED(message) __declspec(deprecated(message))
#else
#define ARK_DEPRECATED(message)
#endif

#ifndef ARK_USE_STATIC_RUNTIME_LIBRARY
#   ifdef ARK_FLAG_BUILDING_FRAMEWORK
#       define ARK_API ARK_API_EXPORT_DECLARATION
#   elif defined(ARK_FLAG_BUILDING_PLUGIN) && defined(ARK_BUILD_STATIC_PLUGINS)
#       define ARK_API
#   else
#       define ARK_API ARK_API_IMPORT_DECLARATION
#   endif
#else
#   define ARK_API
#endif

#if defined(__clang__)
#   define __ARK_FUNCTION__     __PRETTY_FUNCTION__
#   define __ARK_FUNCTION_SIGANTURE__     __PRETTY_FUNCTION__
#elif defined(__GNUC__) || defined(__GNUG__)
#   define __ARK_FUNCTION__     __PRETTY_FUNCTION__
#   define __ARK_FUNCTION_SIGANTURE__     __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define __ARK_FUNCTION__     __FUNCTION__
#   define __ARK_FUNCTION_SIGANTURE__     __FUNCSIG__
#endif

#define ARK_CONCAT_IMPL(x, y) x##y
#define ARK_CONCAT(x, y) ARK_CONCAT_IMPL(x, y)

#define WARN(...)   ark::__message__(ark::__warning__, __ARK_FUNCTION__, nullptr, __VA_ARGS__)
#define FATAL(...)  ark::__message__(ark::__fatal__, __ARK_FUNCTION__, nullptr, __VA_ARGS__)
#define THREAD_CHECK(threadId) __thread_check__(__ARK_FUNCTION__, threadId)
#define CHECK(cond, ...) if(!(cond)) ark::__message__(ark::__fatal__, __ARK_FUNCTION__, #cond, __VA_ARGS__)
#define CHECK_WARN(cond, ...) if(!(cond)) ark::__message__(ark::__warning__, __ARK_FUNCTION__, #cond, __VA_ARGS__)
#define CHECK_WARN_OR_RETURN(cond, ...) while(!(cond)) { ark::__message__(ark::__warning__, __ARK_FUNCTION__, #cond, __VA_ARGS__); return; }
#define TRACE(cond, ...) if(cond) ark::__message__(ark::__trace__, __ARK_FUNCTION__, #cond, __VA_ARGS__)

#ifndef ARK_FLAG_PUBLISHING_BUILD
#   define DPROFILER_TRACE(...) static auto ARK_CONCAT(_ag_tracer_, __LINE__) = Ark::instance().makeProfilerTracer(__ARK_FUNCTION_SIGANTURE__, __FILE__, __LINE__, __VA_ARGS__); \
                                const auto ARK_CONCAT(_ag_scope_, __LINE__) = ARK_CONCAT(_ag_tracer_, __LINE__) ? ARK_CONCAT(_ag_tracer_, __LINE__)->trace() : nullptr
#   define DPROFILER_LOG(name, ...)   static auto ARK_CONCAT(_ag_logger_, __LINE__) = Ark::instance().makeProfilerLogger(__ARK_FUNCTION_SIGANTURE__, __FILE__, __LINE__, name); \
                                      if(ARK_CONCAT(_ag_logger_, __LINE__)) ARK_CONCAT(_ag_logger_, __LINE__)->log(__VA_ARGS__)
#else
#   define DPROFILER_TRACE(...)
#   define DPROFILER_LOG(...)
#endif

#if !defined(ARK_FLAG_PUBLISHING_BUILD) || !defined(ARK_FLAG_BUILD_TYPE) || ARK_FLAG_BUILD_TYPE < 100
#   define DFATAL(...) FATAL(__VA_ARGS__)
#   define DCHECK(cond, ...) CHECK(cond, __VA_ARGS__)
#   define DCHECK_WARN(cond, ...) CHECK_WARN(cond, __VA_ARGS__)
#   define DCHECK_WARN_OR_RETURN(cond, ...) CHECK_WARN_OR_RETURN(cond, __VA_ARGS__)
#   define DTRACE(cond, ...) TRACE(cond, __VA_ARGS__)
#   define DTHREAD_CHECK(threadId) THREAD_CHECK(threadId)
#else
#   define DFATAL(...)
#   define DCHECK(cond, ...) (void (cond))
#   define DCHECK_WARN(cond, ...) (void (cond))
#   define DCHECK_WARN_OR_RETURN(cond, ...) if (!(cond)) return
#   define DTRACE(cond, ...) (void (cond))
#   define DTHREAD_CHECK(threadId) (void (threadId))
#endif

#define ASSERT(x) CHECK(x, "Assertion failed")
#define DASSERT(x) DCHECK(x, "Assertion failed")

namespace ark {

enum THREAD_NAME_ID {
    THREAD_NAME_ID_UNSPECIFIED = 0,
    THREAD_NAME_ID_MAIN = 1,
    THREAD_NAME_ID_CORE = 2,
    THREAD_NAME_ID_RENDERER = 3,
    THREAD_NAME_ID_COUNT = 4
};

typedef void(*fnTraceCallback)(const char* func, const char* condition, const char* message);

void ARK_API __message__(fnTraceCallback callback, const char* func, const char* condition, const char* format = "", ...);
#ifndef ARK_FLAG_BUILD_TYPE
[[noreturn]]
#endif

void ARK_API __fatal__(const char* func, const char* condition, const char* message);
void ARK_API __warning__(const char* func, const char* condition, const char* message);
void ARK_API __trace__(const char* func, const char* condition, const char* message);
bool ARK_API __thread_check__(THREAD_NAME_ID threadNameId);
void ARK_API __thread_check__(const char* func, THREAD_NAME_ID threadId);

void __thread_init__(THREAD_NAME_ID nameId);

template<typename T, size_t N> constexpr size_t array_size(const T (&)[N]) {
    return N - 1;
}

constexpr HashId string_hash(const char* s, const HashId base = 0) {
    constexpr HashId some_prime_number = 101;
    return *s ? (*(s + 1) ? *s + (*(s + 1)) * some_prime_number + (*(s + 2) ? (*(s + 3) ? *(s + 2) + (*(s + 3)) * some_prime_number + string_hash(s + 4, base) * some_prime_number * some_prime_number : *(s + 2)) : base) * some_prime_number * some_prime_number : *s) : base;
}

}
