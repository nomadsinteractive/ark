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
template<typename T> class List;
template<typename T> class SharedPtr;
template<typename T> class Type;
template<typename T> class Variable;
template<typename T> class VariableWrapper;
template<typename T> class WeakPtr;


template <typename T> using sp = SharedPtr<T>;
template <typename T> using op = OwnedPtr<T>;
template <typename T> using array = sp<Array<T>>;

class Ark;
class Attribute;
class BeanFactory;
class Box;
class ByType;
class Changed;
class Class;
class ClassHierarychyManager;
class Clock;
class Document;
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
class StringBuilder;
class StringTable;
class Thread;
class ThreadPoolExecutor;

typedef Dictionary<sp<Readable>> Asset;
typedef Dictionary<sp<String>> StringBundle;

typedef Variable<bool> Boolean;
typedef Variable<int32_t> Integer;
typedef Variable<float> Numeric;

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

typedef Iterator<int32_t> Range;

typedef uint32_t TypeId;
typedef sp<Attribute> attribute;
typedef sp<Element> element;
typedef sp<Document> document;
typedef array<uint8_t> bytearray;
typedef array<float> floatarray;
typedef array<uint16_t> indexarray;

}

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;   \
  void operator=(const TypeName&) = delete

#endif
