#ifndef ARK_CORE_API_H_
#define ARK_CORE_API_H_

#include <limits>
#include <stdint.h>

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

#ifndef ARK_BUILD_STATIC
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
#define THREAD_CHECK(threadId) __thread_check__<threadId>(__ARK_FUNCTION__)
#define CHECK(cond, ...) if(!(cond)) ark::__message__(ark::__fatal__, __ARK_FUNCTION__, #cond, __VA_ARGS__)
#define CHECK_WARN(cond, ...) if(!(cond)) ark::__message__(ark::__warning__, __ARK_FUNCTION__, #cond, __VA_ARGS__)
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

#if ARK_FLAG_DEBUG
#   define DFATAL(...) FATAL(__VA_ARGS__)
#   define DCHECK(cond, ...) CHECK(cond, __VA_ARGS__)
#   define DCHECK_WARN(cond, ...) CHECK_WARN(cond, __VA_ARGS__)
#   define DTRACE(cond) TRACE(cond, __VA_ARGS__)
#   define DTHREAD_CHECK(threadId) THREAD_CHECK(threadId)
#else
#   define DFATAL(...)
#   define DCHECK(cond, ...) (void (cond))
#   define DCHECK_WARN(cond, ...) (void (cond))
#   define DTRACE(cond, ...) (void (cond))
#   define DTHREAD_CHECK(threadId) (void (threadId))
#endif

#define ASSERT(x) CHECK(x, "Assertion failed")
#define DASSERT(x) DCHECK(x, "Assertion failed")

#ifdef ARK_USE_CONSTEXPR
#   define _CONSTEXPR   constexpr
#else
#   define _CONSTEXPR
#endif

namespace ark {

template<typename T> using remove_cv_t = typename std::remove_cv<T>::type;
template<typename T> using remove_reference_t = typename std::remove_reference<T>::type;

template<class T> struct remove_cvref {
    typedef remove_cv_t<remove_reference_t<T>> type;
};

template<typename T> using remove_cvref_t = typename remove_cvref<T>::type;

namespace Constants {

const size_t npos = std::numeric_limits<size_t>::max();

namespace Attributes {

const char* const ID = "id";
const char* const ALPHA = "alpha";
const char* const ALPHABET = "alphabet";
const char* const ATLAS = "atlas";
const char* const BACKGROUND = "background";
const char* const BINDING = "binding";
const char* const BITMAP = "bitmap";
const char* const BOUNDARY = "boundary";
const char* const BOUNDS = "bounds";
const char* const CAMERA = "camera";
const char* const CLASS = "class";
const char* const CLOCK = "clock";
const char* const COLOR = "color";
const char* const DELEGATE = "delegate";
const char* const DISPOSED = "disposed";
const char* const EVENT_LISTENER = "event-listener";
const char* const VARYINGS = "varyings";
const char* const FOREGROUND = "foreground";
const char* const GRAVITY = "gravity";
const char* const LAYOUT = "layout";
const char* const LAYER = "layer";
const char* const MODEL = "model";
const char* const NAME = "name";
const char* const INPUT = "input";
const char* const INTERVAL = "interval";
const char* const POSITION = "position";
const char* const SNIPPET = "snippet";
const char* const SRC = "src";
const char* const RECT = "rect";
const char* const WIDTH = "width";
const char* const HEIGHT = "height";
const char* const DEPTH = "depth";
const char* const REF = "ref";
const char* const RENDERER = "renderer";
const char* const RENDER_OBJECT = "render-object";
const char* const RENDER_LAYER = "render-layer";
const char* const ROTATION = "rotation";
const char* const SHADER = "shader";
const char* const SIZE = "size";
const char* const STYLE = "style";
const char* const TEXT = "text";
const char* const TEXTURE = "texture";
const char* const TIMEOUT = "timeout";
const char* const TRANSFORM = "transform";
const char* const TRANSLATION = "translation";
const char* const TYPE = "type";
const char* const VALUE = "value";
const char* const VIEW = "view";
const char* const VISIBLE = "visible";

const char* const TOP = "top";
const char* const LEFT = "left";
const char* const BOTTOM = "bottom";
const char* const RIGHT = "right";

const char* const TEXT_COLOR = "text-color";
const char* const TEXT_SIZE = "text-size";

const char* const NINE_PATCH_PADDINGS = "paddings";

}
}

enum THREAD_ID {
    THREAD_ID_UNSPECIFIED = 0,
    THREAD_ID_MAIN = 1,
    THREAD_ID_CORE = 2,
    THREAD_ID_RENDERER = 3
};

typedef void(*fnTraceCallback)(const char* func, const char* condition, const char* message);

void ARK_API __message__(fnTraceCallback callback, const char* func, const char* condition, const char* format = "", ...);

[[noreturn]]
void ARK_API __fatal__(const char* func, const char* condition, const char* message);
void ARK_API __warning__(const char* func, const char* condition, const char* message);
void ARK_API __trace__(const char* func, const char* condition, const char* message);

namespace _internal {

template<THREAD_ID ID> struct ThreadFlag {
    static THREAD_ID& id() {
        thread_local THREAD_ID _thread_id = THREAD_ID_UNSPECIFIED;
        return _thread_id;
    }
};

}

template<THREAD_ID ID> void __thread_init__() {
    _internal::ThreadFlag<ID>::id() = ID;
}

template<THREAD_ID ID> void __thread_check__(const char* func) {
    if(_internal::ThreadFlag<ID>::id() != ID)
        __message__(__fatal__, func, "", "ThreadId check failed: %d, should be %d", _internal::ThreadFlag<ID>::id(), ID);
}

bool ARK_API __trace_flag__();
void ARK_API __set_trace_flag__();

}

#endif
