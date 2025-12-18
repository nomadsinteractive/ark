#pragma once

#include <memory>

#include "core/base/api.h"
#include "core/concurrent/non_thread_safe.h"

namespace ark {

template<typename T, typename U, bool DO_THREAD_CHECK> class _TableBase : public NonThreadSafe<DO_THREAD_CHECK> {
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
    _TableBase() = default;
    DEFAULT_COPY_AND_ASSIGN(_TableBase);

    bool has(const T& key) const {
        return _indices.contains(key);
    }

    const Vector<T>& keys() const {
        this->threadCheck();
        return _keys;
    }

    const Vector<U>& values() const {
        this->threadCheck();
        return _values;
    }

    Vector<U>& values() {
        this->threadCheck();
        return _values;
    }

    size_t size() const {
        this->threadCheck();
        return _indices.size();
    }

    bool empty() const {
        return _indices.empty();
    }

    const U& at(const T& key) const {
        this->threadCheck();
        const auto iter = find(key);
        DCHECK(iter != end(), "Key not found");
        return iter->second;
    }

    iterator begin() {
        this->threadCheck();
        return iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    iterator end() {
        return iterator(_keys, _values, npos);
    }

    iterator find(const T& key) {
        this->threadCheck();
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? iterator(_keys, _values, iter->second) : end();
    }

    const_iterator begin() const {
        this->threadCheck();
        return const_iterator(_keys, _values, _keys.empty() ? npos : 0);
    }

    const_iterator end() const {
        return const_iterator(_keys, _values, npos);
    }

    const_iterator find(const T& key) const {
        this->threadCheck();
        const auto iter = _indices.find(key);
        return iter != _indices.end() ? const_iterator(_keys, _values, iter->second) : end();
    }

    template<typename V> Vector<V> flat() const {
        this->threadCheck();
        Vector<V> flatted;
        for(const auto& [k, v] : *this)
            flatted.emplace_back(k, v);
        return flatted;
    }

protected:
    _TableBase(Vector<T> keys, Vector<U> values, IndexType indices)
        : _keys(std::move(keys)), _values(std::move(values)), _indices(std::move(indices)) {
    }

protected:
    Vector<T> _keys;
    Vector<U> _values;
    IndexType _indices;
};

template<typename T, typename U> class Table : public _TableBase<T, U, true> {
public:
    Table() = default;
    Table(std::initializer_list<std::pair<T, U>> items)
    {
        for(auto& [k, v] : items)
            push_back(std::move(k), std::move(v));
    }

    void push_back(T key, U value) {
        this->threadCheck();
        size_t index = this->_values.size();
        this->_indices.insert(std::make_pair(key, index));
        this->_keys.push_back(std::move(key));
        this->_values.push_back(std::move(value));
    }

    U& operator[](const T& key) {
        this->threadCheck();
        const auto iter = this->_indices.find(key);
        if(iter != this->_indices.end())
            return this->_values[iter->second];

        this->_indices.insert(std::make_pair(key, this->_values.size()));
        this->_keys.push_back(key);
        this->_values.push_back(U());
        return this->_values.back();
    }

    void clear() {
        this->threadCheck();
        this->_indices.clear();
        this->_keys.clear();
        this->_values.clear();
    }

    template<typename _T, typename _U> friend class ImmutableTable;
};

template<typename T, typename U> class ImmutableTable : public _TableBase<T, U, false> {
public:
    ImmutableTable(Table<T, U> table)
        : _TableBase<T, U, false>(std::move(table._keys), std::move(table._values), std::move(table._indices)) {
    }
};

}
