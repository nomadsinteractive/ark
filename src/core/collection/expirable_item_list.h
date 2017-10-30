#ifndef ARK_CORE_COLLECTION_EXPIRABLE_ITEM_LIST_H_
#define ARK_CORE_COLLECTION_EXPIRABLE_ITEM_LIST_H_

#include <list>

#include "core/inf/variable.h"
#include "core/epi/expired.h"
#include "core/collection/iterable.h"
#include "core/types/shared_ptr.h"

namespace ark {

template<typename T> class ExpirableItem {
public:
    ExpirableItem(const sp<T>& item)
        : _item(item), _expired(item.template as<Expired>()) {
    }
    ExpirableItem(const sp<T>& item, const sp<Boolean>& expired)
        : _item(item), _expired(expired) {
    }

    ExpirableItem(const ExpirableItem& other)
        : _item(other._item), _expired(other._expired) {
    }

    const sp<T>& item() const {
        return _item;
    }

    sp<T>& item() {
        return _item;
    }

    bool isExpired() const {
        return _expired && _expired->val();
    }

    bool operator < (const ExpirableItem& other) const {
        return _item < other._item;
    }

private:
    sp<T> _item;
    sp<Boolean> _expired;
};

template<typename T> class WeakItem {
public:
    WeakItem(const sp<T>& item)
        : _item(item) {
    }

    WeakItem(const WeakItem& other)
        : _item(other._item) {
    }

    const sp<T>& item() const {
        return _item;
    }

    sp<T>& item() {
        return _item;
    }

    bool isExpired() const {
        return _item.unique();
    }

    bool operator < (const WeakItem& other) const {
        return _item < other._item;
    }

private:
    sp<T> _item;
};

template<typename T, typename U, typename V> class ExpirableItemIteratorBase : public IteratorBase<V> {
public:
    ExpirableItemIteratorBase(V iterator, T& list)
        : IteratorBase<V>::IteratorBase(iterator), _list(list) {
        moveToNext();
    }

    const ExpirableItemIteratorBase<T, U, V>& operator ++ () {
        ++(this->_iterator);
        moveToNext();
        return *this;
    }

    const ExpirableItemIteratorBase<T, U, V> operator ++ (int) {
        T iter = this->_iterator;
        ++(this->_iterator);
        moveToNext();
        return iter;
    }

private:
    void moveToNext() {
        do {
            if(this->_iterator == _list.end())
                break;
            if((*this->_iterator).isExpired())
                this->_iterator = _list.erase(this->_iterator);
            else
                break;
        } while(true);
    }

protected:
    T& _list;
};

template<typename T, typename U> class ExpirableItemConstIterator : public ExpirableItemIteratorBase<T, U, typename T::const_iterator> {
public:
    ExpirableItemConstIterator(typename T::const_iterator iterator, T& list)
        : ExpirableItemIteratorBase<T, U, typename T::const_iterator>::ExpirableItemIteratorBase(iterator, list) {
    }

    const sp<U>& operator * () const {
        return (*this->_iterator).item();
    }
};

template<typename T, typename U> class ExpirableItemIterator : public ExpirableItemIteratorBase<T, U, typename T::iterator> {
public:
    ExpirableItemIterator(typename T::iterator iterator, T& list)
        : ExpirableItemIteratorBase<T, U, typename T::iterator>::ExpirableItemIteratorBase(iterator, list) {
    }

    sp<U>& operator * () {
        return (*this->_iterator).item();
    }
};

template<typename T, typename U> class _ExpirableItemList {
private:
    typedef std::list<U> ItemList;

public:
    typedef ExpirableItemConstIterator<ItemList, T> const_iterator;
    typedef ExpirableItemIterator<ItemList, T> iterator;

    template<typename... Args> void push_back(const sp<T>& resource, Args&&... args) {
        _items.push_back(U(resource, std::forward<Args>(args)...));
    }

    uint32_t size() const {
        return _items.size();
    }

    void clear() {
        _items.clear();
    }

    void remove(const sp<T>& item) {
        for(auto iter = _items.begin(); iter != _items.end(); ++iter) {
            const U& i = *iter;
            if(i.item() == item) {
                _items.erase(iter);
                break;
            }
        }
    }

    const const_iterator begin() const {
        return const_iterator(_items.begin(), _items);
    }

    const const_iterator end() const {
        return const_iterator(_items.end(), _items);
    }

    iterator begin() {
        return iterator(_items.begin(), _items);
    }

    iterator end() {
        return iterator(_items.end(), _items);
    }

private:
    ItemList _items;
};

template <typename T> using ExpirableItemList = _ExpirableItemList<T, ExpirableItem<T>>;
template <typename T> using WeakItemList = _ExpirableItemList<T, WeakItem<T>>;

}

#endif
