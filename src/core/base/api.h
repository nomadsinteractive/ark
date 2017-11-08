#ifndef ARK_CORE_API_H_
#define ARK_CORE_API_H_

#include <map>
#include <thread>

#include <stdint.h>
#include <string>

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

#ifndef ARK_STATIC
#ifdef ARK_FLAG_BUILDING_FRAMEWORK
#   define ARK_API ARK_API_EXPORT_DECLARATION
#else
#   define ARK_API ARK_API_IMPORT_DECLARATION
#endif
#else
#   define ARK_API
#endif

#if defined(__clang__)
#   define __ARK_FUNCTION__     __PRETTY_FUNCTION__
#elif defined(__GNUC__) || defined(__GNUG__)
#   define __ARK_FUNCTION__     __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#   define __ARK_FUNCTION__     __FUNCTION__
#endif

#define FATAL(...) ark::__fatal__(__ARK_FUNCTION__, nullptr, __VA_ARGS__)
#define CHECK(cond, ...) if(!(cond)) ark::__fatal__(__ARK_FUNCTION__, #cond, __VA_ARGS__)
#define WARN(cond, ...) if(!(cond)) ark::__warning__(__ARK_FUNCTION__, __VA_ARGS__)

#ifdef ARK_FLAG_DEBUG
#   define DFATAL(...) FATAL(__VA_ARGS__)
#   define DCHECK(cond, ...) CHECK(cond, __VA_ARGS__)
#   define DWARN(cond, ...) WARN(cond, __VA_ARGS__)
#   define DTRACE(cond) if(cond) __trace__()
#else
#   define DFATAL(...)
#   define DCHECK(cond, ...) (void (cond))
#   define DWARN(cond, ...) (void (cond))
#   define DWARN(cond, ...) (void (cond))
#   define DTRACE(cond) (void (cond))
#endif

#define NOT_NULL(x) DCHECK(x, "Null Pointer Exception")

#ifdef ARK_USE_CONSTEXPR
#   define _CONSTEXPR   constexpr
#else
#   define _CONSTEXPR
#endif

namespace ark {

namespace Constants {
namespace Attributes {

const char* const ID = "id";
const char* const ALPHA = "alpha";
const char* const ALPHABET = "alphabet";
const char* const ATLAS = "atlas";
const char* const BACKGROUND = "background";
const char* const BOUNDARY = "boundary";
const char* const BOUNDS = "bounds";
const char* const CLASS = "class";
const char* const CLOCK = "clock";
const char* const COLOR = "color";
const char* const DELEGATE = "delegate";
const char* const FILTER = "filter";
const char* const FOREGROUND = "foreground";
const char* const EXPIRED = "expired";
const char* const GRAVITY = "gravity";
const char* const LAYOUT = "layout";
const char* const LAYER = "layer";
const char* const MOVIECLIP = "movieclip";
const char* const NAME = "name";
const char* const INTERVAL = "interval";
const char* const PATH = "path";
const char* const POSITION = "position";
const char* const SNIPPET = "snippet";
const char* const SRC = "src";
const char* const RECT = "rect";
const char* const WIDTH = "width";
const char* const HEIGHT = "height";
const char* const REF = "ref";
const char* const RENDER_OBJECT = "render-object";
const char* const RENDER_LAYER = "render-layer";
const char* const SHADER = "shader";
const char* const SIZE = "size";
const char* const STYLE = "style";
const char* const TEXT = "text";
const char* const TIMEOUT = "timeout";
const char* const TRANSFORM = "transform";
const char* const TRANSLATION = "translation";
const char* const TYPE = "type";
const char* const VALUE = "value";
const char* const VIEW = "view";

const char* const TOP = "top";
const char* const LEFT = "left";
const char* const BOTTOM = "bottom";
const char* const RIGHT = "right";

const char* const TEXT_COLOR = "text-color";
const char* const TEXT_SIZE = "text-size";

const char* const ON_CROSS = "oncross";

const char* const NINE_PATCH_PATCHES = "patches";

}
}

void ARK_API __fatal__(const char* func, const char* condition, const char* format, ...);
void ARK_API __warning__(const char* func, const char* format, ...);

uint32_t ARK_API __trace__();

namespace _internal {

template<typename T> class _Properties {
public:
    static std::map<std::string, T> _PROPERTIES;
};

template<typename T> std::map<std::string, T> _Properties<T>::_PROPERTIES;

}

template<typename T> T __prop__(const String& name, const T& defvalue) {
    auto iter = _internal::_Properties<T>::_PROPERTIES.find(name);
    return iter != _internal::_Properties<T>::_PROPERTIES.end() ? iter->second : defvalue;
}

extern bool g_isOriginBottom;
extern float g_upDirection;

extern std::thread::id g_kMainThread;
extern std::thread::id g_kRenderThread;

}

#endif
