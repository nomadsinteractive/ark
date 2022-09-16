#ifndef ARK_CORE_FORWARDING_H_
#define ARK_CORE_FORWARDING_H_

#include <chrono>

#include <stdint.h>
#include <unordered_map>

namespace ark {

template<typename T> class Array;
template<typename T> class Builder;
template<typename T> class ByIndex;
template<typename T> class Callable;
template<typename T> class Delegate;
template<typename T> class Dictionary;
template<typename T> class Duck;
template<typename T> class Expectation;
template<typename T> class Importer;
template<typename T> class Optional;
template<typename T> class Outputer;
template<typename T> class OwnedPtr;
template<typename T> class Loader;
template<typename T> class LoaderBundle;
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
class Allocator;
class Asset;
class AssetBundle;
class BeanFactory;
class BeanFactoryWeakRef;
class Box;
class ByType;
class Class;
class ClassHierarychyManager;
class Clock;
class Command;
class CommandGroup;
class DOMDocument;
class DOMAttribute;
class DOMElement;
class Disposed;
class Duration;
class Executor;
class Input;
class Function;
class Future;
class Holder;
class Interfaces;
class Json;
class Notifier;
class Manifest;
class MemoryPool;
class MessageLoop;
class Mutex;
class Object;
class Observer;
class Plugin;
class PluginManager;
class Queries;
class Readable;
class Runnable;
class Scope;
class Script;
class State;
class StateMachine;
class Storage;
class String;
class StringBundle;
class StringBuffer;
class StringTable;
class Thread;
class ExecutorThreadPool;
class Updatable;
class Visibility;
class Writable;

typedef Dictionary<Box> BoxBundle;

typedef Variable<bool> Boolean;
typedef Variable<int32_t> Integer;
typedef Variable<float> Numeric;
typedef Variable<sp<String>> Text;

typedef VariableWrapper<bool> BooleanWrapper;
typedef VariableWrapper<int32_t> IntegerWrapper;
typedef VariableWrapper<float> NumericWrapper;
typedef VariableWrapper<sp<String>> TextWrapper;

typedef std::unordered_map<int32_t, int32_t> IntMap;
typedef Importer<IntMap> IntMapImporter;

typedef uint16_t element_index_t;

typedef Array<uint8_t>  ByteArray;
typedef Array<int32_t>  IntArray;
typedef Array<element_index_t>  IndexArray;
typedef Array<float>    FloatArray;

typedef uint32_t TypeId;

typedef sp<DOMAttribute> attribute;
typedef sp<DOMElement> element;
typedef sp<DOMDocument> document;
typedef sp<ByteArray> bytearray;
typedef sp<IntArray> intarray;
typedef sp<IndexArray> indexarray;
typedef sp<FloatArray> floatarray;

typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

typedef Loader<document> DocumentLoader;
typedef LoaderBundle<document> DocumentLoaderBundle;

typedef Loader<Json> JsonLoader;
typedef LoaderBundle<Json> JsonLoaderBundle;

typedef Loader<String> StringLoader;
typedef LoaderBundle<String> StringLoaderBundle;

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
