#pragma once

#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "core/base/api.h"

namespace ark {

template<typename T, typename U, bool Ordered> class Table {
public:
    static constexpr size_t npos = std::numeric_limits<size_t>::max();

    template<bool IS_CONSTANT> struct Iterator {

        template <typename V> using PType = std::conditional_t<IS_CONSTANT, const V&, V&>;
        template <typename V> using VType = std::conditional_t<IS_CONSTANT, const std::vector<V>&, std::vector<V>&>;

        typedef std::pair<PType<T>, PType<U>> PairType;

        Iterator(VType<T> keys, VType<U> values, size_t iterator)
            : _keys(keys), _values(values), _iterator(iterator), _data(iterator != npos ? new PairType(_keys.at(_iterator), _values.at(_iterator)) : nullptr) {
            DCHECK(keys.size() == values.size(), "Zipped iterator must be equal length");
        }
        Iterator(Iterator&& other)
            : _keys(other._keys), _values(other._values), _iterator(other._iterator), _data(std::move(other._data)) {
        }
        DISALLOW_COPY_AND_ASSIGN(Iterator);

        Iterator& operator ++() {
            next();
            return *this;
        }

        bool operator == (const Iterator& other) const {
            return &_keys == &other._keys && &_values == &other._values && _iterator == other._iterator;
        }

        bool operator != (const Iterator& other) const {
            return !(*this == other);
        }

        Iterator operator ++(int) {
            next();
            return Iterator(_keys, _values, _iterator - 1);
        }

        const PairType& operator *() const {
            DASSERT(_data);
            return *_data;
        }

        const PairType* operator ->() const {
            DASSERT(_data);
            return _data.get();
        }

    private:
        void next() {
            DASSERT(_data);
            DASSERT(_iterator != npos);
            DASSERT(_iterator < _keys.size());
            ++_iterator;
            if(_iterator == _keys.size())
                _iterator = npos;
            else
                _data.reset(new PairType(_keys.at(_iterator), _values.at(_iterator)));
        }

    private:
        VType<T> _keys;
        VType<U> _values;

        size_t _iterator;
        std::unique_ptr<PairType> _data;
    };

    typedef Iterator<false> iterator;
    typedef Iterator<true>  const_iterator;
    typedef std::conditional_t<Ordered, std::map<T, size_t>, std::unordered_map<T, size_t>> IndexType;

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

    const std::vector<T>& keys() const {
        return _keys;
    }

    const std::vector<U>& values() const {
        return _values;
    }

    std::vector<U>& values() {
        return _values;
    }

    size_t size() const {
        return _indices.size();
    }

    const U& at(const T& key) const {
        const auto iter = find(key);
        DCHECK(iter != end(), "Key not found");
        return iter->second;
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

    iterator begin() {
        return iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    iterator end() {
        return iterator(_keys, _values, npos);
    }

    iterator find(const T& key) {
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? iterator(_keys, _values, iter->second) : end();
    }

    const_iterator begin() const {
        return const_iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    const_iterator end() const {
        return const_iterator(_keys, _values, npos);
    }

    const_iterator find(const T& key) const {
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? const_iterator(_keys, _values, iter->second) : end();
    }

    void clear() {
        _indices.clear();
        _keys.clear();
        _values.clear();
    }

    template<typename V> std::vector<V> flat() const {
        std::vector<V> flatted;
        for(const auto& [k, v] : *this)
            flatted.emplace_back(k, v);
        return flatted;
    }

private:
    std::vector<T> _keys;
    std::vector<U> _values;
    IndexType _indices;
};

}
