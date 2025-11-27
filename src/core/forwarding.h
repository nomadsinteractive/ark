#pragma once

#include <stdint.h>

#include <chrono>
#include <list>
#include <map>
#include <span>
#include <set>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace ark {

template<typename T> class Array;
template<typename T, bool SHIFT = false> class BitSet;
template<typename T> class Dictionary;
template<typename T> class Duck;
template<typename T> class Global;
template<typename T> class IBuilder;
template<typename T> class Importer;
template<typename T> class Optional;
template<typename T> class Outputer;
template<typename T> class OwnedPtr;
template<typename T> class Loader;
template<typename T> class LoaderBundle;
template<typename T> class LFStack;
template<typename T> class LFQueue;
template<typename T> class Range;
template<typename T> class OptionalVar;
template<typename T> class SharedPtr;
template<typename T, typename U> class Table;
template<typename T> class Type;
template<typename T> class Variable;
template<typename T> class VariableWrapper;
template<typename T> class WeakPtr;
template<typename T> class Wrapper;

template <typename T> using Vector = std::vector<T>;
template <typename T> using List = std::list<T>;
template <typename T> using Set = std::set<T>;
template <typename T, typename H = std::hash<T>> using HashSet = std::unordered_set<T, H>;

template <typename T, typename U> using HashMap = std::unordered_map<T, U>;
template <typename T, typename U> using Map = std::map<T, U>;

template <typename T> using sp = SharedPtr<T>;
template <typename T> using op = OwnedPtr<T>;
template <typename T> using array = sp<Array<T>>;

template <typename T> using Builder = IBuilder<SharedPtr<T>>;
template <typename T> using builder = SharedPtr<Builder<T>>;

class Ark;
class Allocator;
class ApplicationContext;
class ApplicationManifest;
class Asset;
class AssetBundle;
class BeanFactory;
class Behavior;
class Box;
class Class;
class Clock;
class DOMDocument;
class DOMAttribute;
class DOMElement;
class Discarded;
class Enum;
class Executor;
class ExecutorScheduled;
class ExecutorThreadPool;
class Uploader;
class Future;
class Debris;
class Interpreter;
class Json;
class Manifest;
class MessageLoop;
class Mutex;
class Observer;
class Plugin;
class PluginManager;
class Readable;
class Runnable;
class Scope;
class Slice;
class State;
class Storage;
class String;
class StringBundle;
class StringBuffer;
class StringTable;
class Tags;
class Thread;
class Traits;
class Ref;
class ResourceLoader;
class Updatable;
class UploaderWrapper;
class Visibility;
class Wirable;
class Writable;

typedef Dictionary<Box> BoxBundle;
typedef std::string_view StringView;

typedef Variable<bool> Boolean;
typedef Variable<int32_t> Integer;
typedef Variable<float> Numeric;
typedef Variable<StringView> StringVar;

typedef VariableWrapper<bool> BooleanWrapper;
typedef VariableWrapper<int32_t> IntegerWrapper;
typedef VariableWrapper<float> NumericWrapper;
typedef VariableWrapper<StringView> StringVarWrapper;

typedef std::unordered_map<int32_t, int32_t> IntMap;
typedef Importer<IntMap> IntMapImporter;

typedef uint16_t element_index_t;

typedef Array<uint8_t>  ByteArray;
typedef Array<int32_t>  IntArray;
typedef Array<element_index_t>  IndexArray;
typedef Array<float>    FloatArray;

typedef uint32_t RefId;
typedef uint32_t HashId;

struct TypeId {
    constexpr TypeId()
        : _hash(0) {
    }
    constexpr TypeId(const HashId hash)
        : _hash(hash) {
    }

    constexpr operator HashId() const {
        return _hash;
    }

    constexpr bool operator < (const TypeId other) const {
        return _hash < other._hash;
    }

    HashId _hash;
};

typedef sp<DOMAttribute> attribute;
typedef sp<DOMElement> element;
typedef sp<DOMDocument> document;
typedef sp<ByteArray> bytearray;
typedef sp<IntArray> intarray;
typedef sp<IndexArray> indexarray;
typedef sp<FloatArray> floatarray;

typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
typedef std::span<uint8_t> BytesView;

typedef Loader<document> DocumentLoader;
typedef LoaderBundle<document> DocumentLoaderBundle;

typedef Loader<Json> JsonLoader;
typedef LoaderBundle<Json> JsonLoaderBundle;

typedef Loader<String> StringLoader;
typedef LoaderBundle<String> StringLoaderBundle;

}

template <> struct std::hash<ark::TypeId> {
    size_t operator()(const ark::TypeId typeId) const {
        return typeId._hash;
    }
};


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
