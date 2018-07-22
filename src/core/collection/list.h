#ifndef ARK_CORE_COLLECTION_LIST_H_
#define ARK_CORE_COLLECTION_LIST_H_

#include <list>

#include "core/collection/iterable.h"

namespace ark {

template<typename T> class List : public Iterable<typename std::list<T>> {
public:
    List()
        : Iterable<typename std::list<T>>::Iterable(_items) {
    }
    List(const List<T>& other)
        : Iterable<typename std::list<T>>::Iterable(_items), _items(other._items) {
    }
    List(List<T>&& other)
        : Iterable<typename std::list<T>>::Iterable(_items), _items(std::move(other._items)) {
    }

    typedef typename std::list<T>::iterator iterator;

    const List& operator =(const List<T>& other) {
        _items = other._items;
        return *this;
    }

    const List& operator =(List<T>&& other) {
        _items = std::move(other._items);
        return *this;
    }

    void push_front(const T& item) {
        _items.push_front(item);
    }

    void push_back(const T& item) {
        _items.push_back(item);
    }

    iterator erase(iterator iter) {
        return _items.erase(iter);
    }

    void clear() {
        _items.clear();
    }

    bool isEmpty() const {
        return _items.size() == 0;
    }

    const std::list<T>& items() const {
        return _items;
    }

    std::list<T>& items() {
        return _items;
    }

    size_t size() const {
        return _items.size();
    }

    static List<T>& emptyList() {
        static List<T> l;
        return l;
    }

private:
    std::list<T> _items;
};

}

#endif
