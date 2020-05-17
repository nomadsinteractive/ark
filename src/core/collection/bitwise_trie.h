#ifndef ARK_CORE_BITWISE_TRIE_H_
#define ARK_CORE_BITWISE_TRIE_H_

#include <algorithm>
#include <map>
#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"

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
        Key(KeyType value, uint32_t level)
            : _mask(static_cast<KeyType>(1) << (MAX_LEVEL_DEPTH - 1 - level)), _prefix(value >> (MAX_LEVEL_DEPTH - 1 - level)), _value(value) {
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
            : Path(0, 0, nullptr) {
        }
        Path(KeyType key, uint32_t level, Path* upper)
            : _key(key), _level(level), _upper(upper), _next{nullptr, nullptr}, _leaf(nullptr) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Path);

        bool isLeaf() const {
            return _next[0] == nullptr && _next[1] == nullptr;
        }

        const Path* findLeafPath(const Key& key, uint32_t minEntry = N) const {
            if(isLeaf())
                return _key >= key._value ? this : nullptr;

            uint32_t sig = key._prefix & 1;
            const Path* next = _next[sig] ? _next[sig] : _next[std::max<uint32_t>(minEntry, sig ? 0 : 1)];
            return next ? next->findLeafPath(key.shift(), next != _next[sig] ? 0 : minEntry) : nullptr;
        }

        void removeNode() {
            _leaf = nullptr;
            if(_upper)
                _upper->_next[_upper->_next[0] == this ? 0 : 1] = nullptr;
        }

        KeyType _key;
        uint32_t _level;

        Path* _upper;
        Path* _next[2];

        U* _leaf;
    };

    struct LSS {

        Path* ensureNextRoute(const Key& key, uint32_t level, Path* upper) {
            uint32_t sig = key._prefix & 1;
            if(!upper->_next[sig])
                upper->_next[sig] = addPath(key, level, upper);
            return upper->_next[sig];
        }

        Path* addPath(const Key& key, uint32_t level, Path* upper) {
            Path& path = _paths[key._prefix];
            path = Path(key._value, level, upper);
            return &path;
        }

        std::map<KeyType, Path> _paths;
    };

public:
    BitwiseTrie() = default;
    DEFAULT_COPY_AND_ASSIGN(BitwiseTrie);

    U* put(T key, U value) {
        Path* path = makeLeafPath(&_root, Key(static_cast<KeyType>(key), 0), 0);
        U& leaf = _leaves[key];
        leaf = std::move(value);
        path->_leaf = &leaf;
        return &leaf;
    }

    U* ensure(T key) {
        Path* path = makeLeafPath(&_root, Key(static_cast<KeyType>(key), 0), 0);
        U& leaf = _leaves[key];
        path->_leaf = &leaf;
        return &leaf;
    }

    U* find(T keyvalue) const {
        const Path* path = findLeafPath(keyvalue);
        return path ? path->_leaf : nullptr;
    }

    void remove(T keyvalue) {
        const auto iter = _leaves.find(keyvalue);
        DWARN(iter != _leaves.end(), "Key \"%d\" does not exist", keyvalue);
        if(iter != _leaves.end()) {
            Path* path = const_cast<Path*>(findLeafPath(keyvalue));
            DASSERT(path && path->_key == keyvalue);
            do {
                auto& paths = _lss[path->_level]._paths;
                KeyType prefix = path->_key == keyvalue ? keyvalue >> (MAX_LEVEL_DEPTH - 1 - path->_level) : path->_key;
                const auto it2 = paths.find(prefix);
                DCHECK(it2 != paths.end() && path == &it2->second, "Cannot find prefix %d in level %d", prefix, path->_level);
                path->removeNode();
                path = path->_upper;
                paths.erase(it2);
            } while(path && path->_upper && path->isLeaf());
            _leaves.erase(iter);
        }
    }

    void clear() {
        _root = Path();
        for(uint32_t i = 0; i < MAX_LEVEL_DEPTH; ++i)
            _lss[i]._paths.clear();
        _leaves.clear();
    }

private:
    const Path* findLeafPath(KeyType keyvalue) const {
        uint32_t level;
        Key key(keyvalue, 0);
        const Path* path = findLSS(key, level)->findLeafPath(key);
        if(path) {
            DASSERT(path->_leaf);
            return path;
        }

        for(uint32_t i = level; i > 0; --i) {
            const auto& paths = _lss[i - 1]._paths;
            key.unshift();
            auto iter = paths.find(key._prefix);
            DCHECK(iter != paths.end(), "No prefix \"%d\" in level %d", key._prefix, level);
            if((++iter) != paths.end())
                return iter->second.findLeafPath(key, 0);
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

    Path* makeLeafPath(Path* upper, Key key, uint32_t level) {
        Path* path = upper;

        for(size_t i = level; i < MAX_LEVEL_DEPTH; ++i) {
            path = _lss[i].ensureNextRoute(key, i, path);
            if(path->isLeaf()) {
                if(path->_key != key._value) {
                    Key c(path->_key, i);
                    U* leaf = path->_leaf;

                    path->_key = key._prefix;
                    path->_leaf = nullptr;

                    for(size_t j = i + 1; j < MAX_LEVEL_DEPTH; ++j) {
                        c.shift();
                        key.shift();
                        if(c._prefix != key._prefix) {
                            makeLeafPath(path, c, j)->_leaf = leaf;
                            return makeLeafPath(path, key, j);
                        }
                        path = _lss[j].ensureNextRoute(key, j, path);
                        path->_key = key._prefix;
                    }
                }
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
