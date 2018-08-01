#ifndef ARK_CORE_COLLECTION_TABLE_H_
#define ARK_CORE_COLLECTION_TABLE_H_

#include <unordered_map>
#include <vector>

#include "core/base/api.h"

namespace ark {

template<typename T, typename U> class Table {
public:
    Table() = default;
    DEFAULT_COPY_AND_ASSIGN(Table);

    void push_back(T key, U value) {
        size_t index = _values.size();
        _keys.insert(std::make_pair(std::move(key), index));
        _values.push_back(std::move(value));
    }

    bool has(const T& key) const {
        return _keys.find(key) != _keys.end();
    }

    size_t find(const T& key) const {
        const auto iter = _keys.find(key);
        return iter != _keys.end() ? iter->second : Constants::npos;
    }

    const std::unordered_map<T, size_t>& keys() const {
        return _keys;
    }

    const std::vector<U>& values() const {
        return _values;
    }

    const U& at(const T& key) const {
        size_t index = find(key);
        DCHECK(index != Constants::npos, "Key not found");
        DCHECK(index < _values.size(), "Index out of range");
        return _values[index];
    }

    U& at(const T& key) {
        size_t index = find(key);
        DCHECK(index != Constants::npos, "Key not found");
        DCHECK(index < _values.size(), "Index out of range");
        return _values[index];
    }

    const U& operator[](size_t index) const {
        return _values[index];
    }

    U& operator[](size_t index) {
        return _values[index];
    }

private:
    std::unordered_map<T, size_t> _keys;
    std::vector<U> _values;
};

}

#endif
