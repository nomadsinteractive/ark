#ifndef ARK_CORE_BITWISE_TRIE_H_
#define ARK_CORE_BITWISE_TRIE_H_

#include <limits>
#include <map>
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
    typedef typename _KeySelector<sizeof(T)>::KeyValue KeyType;
    static_assert (N < 2, "Default lookup position must be lesser than 2");
    static_assert(std::is_convertible<T, KeyType>::value, "Key value must be unsigned integer or can be casted into an unsigned integer");

    enum {
        KEY_SIZE = sizeof(KeyType),
        MAX_LEVEL_DEPTH = KEY_SIZE << 3
    };

    struct Key {
        Key(KeyType value, uint32_t level = 0)
            : _mask(1 << (MAX_LEVEL_DEPTH - 1 - level)), _prefix(((std::numeric_limits<KeyType>::max() >> (level + 1)) & value) >> (MAX_LEVEL_DEPTH - 2 - level)), _value(value) {
        }
        Key(KeyType mask, KeyType prefix, KeyType value)
            : _mask(mask), _prefix(prefix), _value(value) {
        }

        void shift() {
            _prefix <<= 1;
            _mask >>= 1;
            _prefix |= (_value & _mask ? 1 : 0);
        }

        Key shift() const {
            return Key(_mask >> 1, (_prefix << 1) | (_value & (_mask >> 1) ? 1 : 0), _value);
        }

        void unshift() {
            _prefix >>= 1;
            _mask <<= 1;
        }

        KeyType _mask;
        KeyType _prefix;
        KeyType _value;
    };

    struct Path {
        Path()
            : Path(0, nullptr) {
        }
        Path(KeyType key, Path* upper)
            : _key(key), _upper(upper), _next{nullptr, nullptr}, _leaf(nullptr) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Path);

        bool isLeaf() const {
            return _next[0] == nullptr && _next[1] == nullptr;
        }

        const Path* findLeafPath(const Key& key) const {
            if(isLeaf())
                return _key >= key._value ? this : nullptr;

            uint32_t sig = key._prefix & 1;
            const Path* next = _next[sig] ? _next[sig] : _next[N];
            return next ? next->findLeafPath(key.shift()) : nullptr;
        }

        KeyType _key;

        Path* _upper;
        Path* _next[2];

        U* _leaf;
    };

    struct LSS {

        Path* ensureNextRoute(KeyType prefix, Path* upper) {
            uint32_t sig = prefix & 1;
            if(!upper->_next[sig])
                upper->_next[sig] = addPath(prefix, upper);
            return upper->_next[sig];
        }

        Path* addPath(KeyType key, Path* upper) {
            Path& path = _paths[key];
            path = Path(key, upper);
            return &path;
        }

        std::map<KeyType, Path> _paths;
    };

public:
    BitwiseTrie() = default;
    DEFAULT_COPY_AND_ASSIGN(BitwiseTrie);

    U* put(T key, U value) {
        Path* path = makeLeafPath(&_root, static_cast<KeyType>(key));
        U& leaf = _leaves[key];
        leaf = std::move(value);
        path->_leaf = &leaf;
        return &leaf;
    }

    U* find(T keyvalue) const {
        const Path* path = findLeafPath(keyvalue);
        return path ? path->_leaf : nullptr;
    }

private:
    const Path* findLeafPath(KeyType keyvalue) const {
        uint32_t level;
        Key key(keyvalue);
        const Path* path = findLSS(key, level)->findLeafPath(key);
        if(path)
            return path;

        for(uint32_t i = level; i > 0; --i) {
            const auto& paths = _lss[i - 1]._paths;
            key.unshift();
            auto iter = paths.find(key._prefix);
            DCHECK(iter != paths.end(), "No prefix \"%d\" in level %d", key._prefix, level);
            if((++iter) != paths.end())
                return iter->second.findLeafPath(key);
        }
        return nullptr;
    }

    const Path* findLSS(Key& key, uint32_t& level) const {
        const Path* path = &_root;
        level = 0;
        for(uint32_t i = 0; i < MAX_LEVEL_DEPTH; ++i) {
            uint32_t sig = key._prefix & 1;
            if(path->_next[sig]) {
                path = path->_next[sig];
                ++level;
                key.shift();
            } else
                break;
        }
        return path;
    }

    Path* makeLeafPath(Path* root, KeyType keyvalue) {
        Key key(keyvalue);
        Path* path = root;

        for(size_t i = 0; i < MAX_LEVEL_DEPTH; ++i) {
            path = _lss[i].ensureNextRoute(key._prefix, path);
            if(key._prefix && path->isLeaf()) {
                path->_key = keyvalue;
                break;
            }
            key.shift();
        }
        return path;
    }

private:
    Path _root;
    LSS _lss[MAX_LEVEL_DEPTH];
    std::unordered_map<KeyType, U> _leaves;
};

}

#endif
