#ifndef ARK_CORE_COLLECTION_ITERABLE_H_
#define ARK_CORE_COLLECTION_ITERABLE_H_

#include <map>

namespace ark {

template<typename T, typename U = typename T::const_iterator, typename V = typename T::iterator> class Iterable {
public:
    Iterable(T& items)
        : _items(&items) {
    }

    Iterable(const Iterable& other)
        : _items(other._items) {
    }

    typedef U const_iterator;
    typedef V iterator;

    const const_iterator begin() const {
        return const_iterator(_items->begin());
    }

    const const_iterator end() const {
        return const_iterator(_items->end());
    }

    iterator begin() {
        return iterator(_items->begin());
    }

    iterator end() {
        return iterator(_items->end());
    }

    Iterable<T, U, V>& operator = (const Iterable<T, U, V>& other) {
        _items = other._items;
        return *this;
    }

protected:
    T* _items;
};

template<typename T> class IteratorBase {
public:
    IteratorBase(T iterator)
        : _iterator(std::move(iterator)) {
    }

    bool operator == (const IteratorBase<T>& other) {
        return _iterator == other._iterator;
    }

    bool operator != (const IteratorBase<T>& other) {
        return _iterator != other._iterator;
    }

    const IteratorBase<T>& operator ++ () {
        ++_iterator;
        return *this;
    }

    const IteratorBase<T> operator ++ (int) {
        T iter = _iterator;
        ++_iterator;
        return iter;
    }

protected:
    T _iterator;
};

template<typename T, typename U> class MapValueIterator : public IteratorBase<T> {
public:
    MapValueIterator(T iterator)
        : IteratorBase<T>::IteratorBase(iterator) {
    }

    U& operator * () {
        return this->_iterator->second;
    }
};

template<typename T, typename U> class MapValueConstIterator : public IteratorBase<T> {
public:
    MapValueConstIterator(T iterator)
        : IteratorBase<T>::IteratorBase(iterator) {
    }

    const U& operator * () const {
        return this->_iterator->second;
    }
};

template<typename K, typename V> using MapValueIterable = Iterable<typename std::map<K, V>, MapValueConstIterator<typename std::map<K, V>::const_iterator, V>, MapValueIterator<typename std::map<K, V>::iterator, V>>;

}

#endif
