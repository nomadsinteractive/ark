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
template<typename T> class Iterator;
template<typename T> class OwnedPtr;
template<typename T> class Loader;
template<typename T> class List;
template<typename T> class LockFreeStack;
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
class Attribute;
class Asset;
class BeanFactory;
class Box;
class ByType;
class Changed;
class Class;
class ClassHierarychyManager;
class Clock;
class Document;
class Duration;
class Element;
class Executor;
class Expectation;
class Expired;
class Flatable;
class Function;
class Future;
class Interfaces;
class MemoryPool;
class MessageLoop;
class Object;
class ObjectPool;
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

typedef Dictionary<sp<String>> StringBundle;

typedef Variable<bool> Boolean;
typedef Variable<int32_t> Integer;
typedef Variable<float> Numeric;

typedef Loader<Document> DocumentLoader;


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

typedef Array<uint8_t>  ByteArray;
typedef Array<int32_t>  IntArray;
typedef Array<float>    FloatArray;

typedef uint32_t TypeId;
typedef sp<Attribute> attribute;
typedef sp<Element> element;
typedef sp<Document> document;
typedef sp<ByteArray> bytearray;
typedef sp<FloatArray> floatarray;

typedef uint16_t glindex_t;
typedef array<glindex_t> indexarray;

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
