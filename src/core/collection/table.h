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
        _indices.insert(std::make_pair(key, index));
        _keys.push_back(std::move(key));
        _values.push_back(std::move(value));
    }

    bool has(const T& key) const {
        return _indices.find(key) != _indices.end();
    }

    size_t find(const T& key) const {
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? iter->second : Constants::npos;
    }

    const std::vector<T>& keys() const {
        return _keys;
    }

    const std::vector<U>& values() const {
        return _values;
    }

    size_t size() const {
        return _indices.size();
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

    U& operator[](const T& key) {
        const auto iter = _indices.find(key);
        if(iter != _indices.end())
            return _values[iter->second];
        _indices.insert(std::make_pair(key, _values.size()));
        _keys.push_back(key);
        _values.push_back(U());
        return _values.back();
    }

    template<typename TIter, typename UIter> struct ZippedIterator {
        TIter _key_iter;
        UIter _value_iter;

        ZippedIterator(TIter keyIter, UIter valueIter)
            : _key_iter(std::move(keyIter)), _value_iter(std::move(valueIter)) {
        }

        ZippedIterator& operator ++() {
            ++(this->_key_iter);
            ++(this->_value_iter);
            return *this;
        }

        bool operator == (const ZippedIterator<TIter, UIter>& other) const {
            bool equal = this->_key_iter == other._key_iter;
            DCHECK(equal == (this->_value_iter == other._value_iter), "Zipped iterator must be both equal or neither");
            return equal;
        }

        bool operator != (const ZippedIterator<TIter, UIter>& other) const {
            return !(*this == other);
        }

        ZippedIterator operator ++(int) {
            return ZippedIterator(_key_iter++, _value_iter++);
        }

        std::pair<T, U> operator *() const {
            return std::pair<T, U>(*_key_iter, *_value_iter);
        }
    };

    struct Zipped {
        const std::vector<T>& _keys;
        const std::vector<U>& _values;

        Zipped(const std::vector<T>& keys, const std::vector<U>& values)
            : _keys(keys), _values(values) {
        }

        ZippedIterator<typename std::vector<T>::const_iterator, typename std::vector<U>::const_iterator> begin() const {
            return ZippedIterator<typename std::vector<T>::const_iterator, typename std::vector<U>::const_iterator>(_keys.begin(), _values.begin());
        }

        ZippedIterator<typename std::vector<T>::const_iterator, typename std::vector<U>::const_iterator> end() const {
            return ZippedIterator<typename std::vector<T>::const_iterator, typename std::vector<U>::const_iterator>(_keys.end(), _values.end());
        }
    };

    Zipped items() const {
        return Zipped(_keys, _values);
    }

    template<typename V> V toMap() const {
        V map;
        for(const auto& i : _indices)
            map[i.first] = _values[i.second];
        return map;
    }

private:
    std::unordered_map<T, size_t> _indices;
    std::vector<T> _keys;
    std::vector<U> _values;
};

}

#endif
