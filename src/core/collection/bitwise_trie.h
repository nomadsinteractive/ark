#ifndef ARK_CORE_BITWISE_TRIE_H_
#define ARK_CORE_BITWISE_TRIE_H_

#include <unordered_map>

#include "core/forwarding.h"

namespace ark {

namespace {

template<size_t> struct _KeySelector;
template<> struct _KeySelector<1> { typedef uint8_t KeyValue; };
template<> struct _KeySelector<2> { typedef uint16_t KeyValue; };
template<> struct _KeySelector<4> { typedef uint32_t KeyValue; };
template<> struct _KeySelector<8> { typedef uint64_t KeyValue; };

}

template<typename T, typename U, size_t N = 1> class BitwiseTrie {
private:
    typedef typename _KeySelector<sizeof(T)>::KeyValue KeyValue;
    static_assert (N < 2, "Default lookup position must be lesser than 2");
    static_assert(std::is_convertible<T, KeyValue>::value, "Key value must be unsigned integer or can be casted into an unsigned integer");

    enum {
        KEY_SIZE = sizeof(KeyValue),
        MAX_LEVEL_DEPTH = KEY_SIZE << 3
    };

    struct Key {
        Key(KeyValue value)
            : _shift(MAX_LEVEL_DEPTH - 1), _prefix(value & (1 << _shift) ? 1 : 0), _value(value) {
        }

        KeyValue prefix() const {
            return _prefix;
        }

        KeyValue mask() const {
            return 1 << _shift;
        }

        void shift() {
            _prefix <<= 1;
            --_shift;
            _prefix |= (_value & (1 << _shift) ? 1 : 0);
        }

        void unshift() {
            _prefix >>= 1;
            ++_shift;
        }

        uint32_t _shift;

        KeyValue _prefix;
        KeyValue _value;
    };

    struct Path {
        Path()
            : Path(0, nullptr) {
        }
        Path(KeyValue key, Path* upper)
            : _key(key), _upper(upper), _next{nullptr, nullptr}, _leaf(nullptr) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Path);

        bool isLeave() const {
            return _next[0] == nullptr && _next[1] == nullptr;
        }

        const Path* findLeafPath(KeyValue bitmask, KeyValue bitset) const {
            if(isLeave())
                return this;

            uint32_t sig = (bitset <= _key ? 0 : 1);
            bitset <<= 1;
            bitset |= sig;

            const Path* next = _next[sig] ? _next[sig] : _next[N];
            return next ? next->findLeafPath(bitmask >> 1, bitset) : nullptr;
        }

        KeyValue _key;

        Path* _upper;
        Path* _next[2];

        U* _leaf;
    };

    struct LSS {

        Path* ensureNextRoute(KeyValue key, Path* upper) {
            uint32_t sig = key <= upper->_key ? 0 : 1;
            if(!upper->_next[sig])
                upper->_next[sig] = addPath(key, upper);
            return upper->_next[sig];
        }

        Path* addPath(KeyValue key, Path* upper) {
            Path& path = _paths[key];
            path = Path(key, upper);
            return &path;
        }

        std::unordered_map<KeyValue, Path> _paths;
    };

public:
    BitwiseTrie() = default;
    DEFAULT_COPY_AND_ASSIGN(BitwiseTrie);

    void put(T key, U value) {
        Path* path = makeLeafPath(&_root, static_cast<KeyValue>(key));
        U& leaf = _leaves[key];
        leaf = std::move(value);
        path->_leaf = &leaf;
    }

    U* find(T key) const {
        const Path* path = findLeafPath(&_root, static_cast<KeyValue>(key));
        return path ? path->_leaf : nullptr;
    }

private:
    const Path* findLeafPath(const Path* root, KeyValue keyvalue) const {
        Key key(keyvalue);
        const Path* path = root;

        for(size_t i = 0; i < MAX_LEVEL_DEPTH; ++i) {
            const auto iter = _lss[i]._paths.find(key._prefix);
            if(iter == _lss[i]._paths.end()) {
                key.unshift();
                break;
            }
            key.shift();
            path = &iter->second;
        }
        return path->findLeafPath(key.mask(), key._prefix);
    }

    Path* makeLeafPath(Path* root, KeyValue keyvalue) {
        Key key(keyvalue);
        Path* path = root;

        for(size_t i = 0; i < MAX_LEVEL_DEPTH; ++i) {
            path = _lss[i].ensureNextRoute(key._prefix, path);
            if(key._prefix && path->isLeave())
                break;
            key.shift();
        }
        return path;
    }

private:
    Path _root;
    LSS _lss[MAX_LEVEL_DEPTH];
    std::unordered_map<KeyValue, U> _leaves;
};

}

#endif
