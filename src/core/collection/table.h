#pragma once

#include <memory>

#include "core/base/api.h"
#include "core/concurrent/non_thread_safe.h"

namespace ark {

template<typename T, typename U> class Table : public NonThreadSafe<THREAD_NAME_ID_CORE> {
public:
    static constexpr size_t npos = std::numeric_limits<size_t>::max();

    typedef T key_type;
    typedef U mapped_type;

    template<bool IS_CONSTANT> struct Iterator {

        template <typename V> using PType = std::conditional_t<IS_CONSTANT, const V&, V&>;
        template <typename V> using VType = std::conditional_t<IS_CONSTANT, const Vector<V>&, Vector<V>&>;

        typedef std::pair<PType<T>, PType<U>> PairType;

        Iterator(VType<T> keys, VType<U> values, const size_t iterator)
            : _keys(keys), _values(values), _iterator(iterator), _data(iterator != npos ? new PairType(_keys.at(_iterator), _values.at(_iterator)) : nullptr) {
            CHECK(keys.size() == values.size(), "Zipped iterator must be equal length: %zu vs %zu", keys.size(), values.size());
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
    typedef Map<T, size_t>  IndexType;

public:
    Table() = default;
    DEFAULT_COPY_AND_ASSIGN(Table);

    void push_back(T key, U value) {
        safetyCheck();
        size_t index = _values.size();
        _indices.insert(std::make_pair(key, index));
        _keys.push_back(std::move(key));
        _values.push_back(std::move(value));
    }

    bool has(const T& key) const {
        return _indices.find(key) != _indices.end();
    }

    const Vector<T>& keys() const {
        safetyCheck();
        return _keys;
    }

    const Vector<U>& values() const {
        safetyCheck();
        return _values;
    }

    Vector<U>& values() {
        safetyCheck();
        return _values;
    }

    size_t size() const {
        safetyCheck();
        return _indices.size();
    }

    bool empty() const {
        return _indices.empty();
    }

    const U& at(const T& key) const {
        safetyCheck();
        const auto iter = find(key);
        DCHECK(iter != end(), "Key not found");
        return iter->second;
    }

    U& operator[](const T& key) {
        safetyCheck();
        const auto iter = _indices.find(key);
        if(iter != _indices.end())
            return _values[iter->second];
        _indices.insert(std::make_pair(key, _values.size()));
        _keys.push_back(key);
        _values.push_back(U());
        return _values.back();
    }

    iterator begin() {
        safetyCheck();
        return iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    iterator end() {
        return iterator(_keys, _values, npos);
    }

    iterator find(const T& key) {
        safetyCheck();
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? iterator(_keys, _values, iter->second) : end();
    }

    const_iterator begin() const {
        safetyCheck();
        return const_iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    const_iterator end() const {
        return const_iterator(_keys, _values, npos);
    }

    const_iterator find(const T& key) const {
        safetyCheck();
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? const_iterator(_keys, _values, iter->second) : end();
    }

    void clear() {
        safetyCheck();
        _indices.clear();
        _keys.clear();
        _values.clear();
    }

    template<typename V> Vector<V> flat() const {
        safetyCheck();
        Vector<V> flatted;
        for(const auto& [k, v] : *this)
            flatted.emplace_back(k, v);
        return flatted;
    }

private:
    Vector<T> _keys;
    Vector<U> _values;
    IndexType _indices;
};

}
