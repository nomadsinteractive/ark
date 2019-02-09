#ifndef ARK_CORE_FORWARDING_H_
#define ARK_CORE_FORWARDING_H_

#include <stdint.h>

namespace ark {

template<typename T> class Array;
template<typename T> class Builder;
template<typename T> class ByIndex;
template<typename T> class Callable;
template<typename T> class Dictionary;
template<typename T> class Duck;
template<typename T> class OwnedPtr;
template<typename T> class Loader;
template<typename T> class List;
template<typename T> class LFStack;
template<typename T> class LFQueue;
template<typename T> class SafePtr;
template<typename T> class SharedPtr;
template<typename T, typename U> class Table;
template<typename T> class Type;
template<typename T> class Variable;
template<typename T> class VariableWrapper;
template<typename T> class WeakPtr;


template <typename T> using sp = SharedPtr<T>;
template <typename T> using op = OwnedPtr<T>;
template <typename T> using array = sp<Array<T>>;

class Ark;
class Asset;
class AssetBundle;
class BeanFactory;
class Box;
class ByType;
class Class;
class ClassHierarychyManager;
class Clock;
class DOMDocument;
class DOMAttribute;
class DOMElement;
class Duration;
class Executor;
class Expectation;
class Flatable;
class Function;
class Future;
class Interfaces;
class Lifecycle;
class Notifier;
class Manifest;
class MemoryPool;
class MessageLoop;
class MessageLoopDefault;
class Object;
class ObjectPool;
class Observer;
class Plugin;
class PluginManager;
class Readable;
class Runnable;
class Scope;
class Script;
class String;
class StringBuffer;
class StringTable;
class Thread;
class ThreadPoolExecutor;
class Visibility;

typedef Dictionary<sp<String>> StringBundle;
typedef Dictionary<Box> BoxBundle;

typedef Variable<bool> Boolean;
typedef Variable<int32_t> Integer;
typedef Variable<float> Numeric;

typedef Loader<DOMDocument> DocumentLoader;


typedef VariableWrapper<bool> BooleanWrapper;
/*
[[core::class]]
class BooleanWrapper : public Boolean {
};
*/

typedef VariableWrapper<int32_t> IntegerWrapper;
/*
[[core::class]]
class IntegerWrapper : public Integer {
};
*/

typedef VariableWrapper<float> NumericWrapper;
/*
[[core::class]]
class NumericWrapper : public Numeric {
};
*/

typedef uint16_t glindex_t;

typedef Array<uint8_t>  ByteArray;
typedef Array<int32_t>  IntArray;
typedef Array<glindex_t>  IndexArray;
typedef Array<float>    FloatArray;

typedef uint32_t TypeId;

typedef sp<DOMAttribute> attribute;
typedef sp<DOMElement> element;
typedef sp<DOMDocument> document;
typedef sp<ByteArray> bytearray;
typedef sp<IntArray> intarray;
typedef sp<IndexArray> indexarray;
typedef sp<FloatArray> floatarray;

}

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;   \
    TypeName& operator=(const TypeName&) = delete

#define DEFAULT_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = default;   \
    TypeName(TypeName&&) = default;   \
    TypeName& operator=(const TypeName&) = default;   \
    TypeName& operator=(TypeName&&) = default

#define DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(TypeName) \
    TypeName(const TypeName&) noexcept = default;   \
    TypeName(TypeName&&) noexcept = default;   \
    TypeName& operator=(const TypeName&) noexcept = default;   \
    TypeName& operator=(TypeName&&) noexcept = default

#endif
