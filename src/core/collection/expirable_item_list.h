#ifndef ARK_CORE_COLLECTION_EXPIRABLE_ITEM_LIST_H_
#define ARK_CORE_COLLECTION_EXPIRABLE_ITEM_LIST_H_

#include <list>

#include "core/inf/variable.h"
#include "core/epi/expired.h"
#include "core/collection/iterable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Validator {
public:

    template<typename T> class IsExpired {
    public:
        IsExpired(const sp<T>& item)
            : _expired(item.as<Expired>()) {
        }
        IsExpired(const sp<T>& item, const sp<Boolean>& expired)
            : _expired(expired) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(IsExpired);

        bool operator()(const sp<T>& /*item*/) const {
            return _expired && _expired->val();
        }

    private:
        sp<Boolean> _expired;
    };


    template<typename T> class IsUnique {
    public:
        IsUnique(const sp<T>& /*item*/) {
        }

        bool operator()(const sp<T>& item) const {
            return item.unique();
        }
    };
};


template<typename T, typename V> struct _ItemWithValidator {
    _ItemWithValidator(const sp<T>& item, V validator)
        : _item(item), _validator(validator) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(_ItemWithValidator);

    bool isExpired() const {
        return _validator(_item);
    }

    bool operator < (const _ItemWithValidator& other) const {
        return _item < other._item;
    }

    sp<T> _item;
    V _validator;
};


template<typename T, typename U> class ExpirableItemIteratorBase : public IteratorBase<U> {
public:
    ExpirableItemIteratorBase(T& list, U iterator)
        : IteratorBase<U>(iterator), _list(list) {
        moveToNext();
    }

    const ExpirableItemIteratorBase<T, U>& operator ++ () {
        ++(this->_iterator);
        moveToNext();
        return *this;
    }

    const ExpirableItemIteratorBase<T, U> operator ++ (int) {
        U iter = this->_iterator;
        ++(this->_iterator);
        moveToNext();
        return iter;
    }

private:
    void moveToNext() {
        do {
            if(this->_iterator == _list.end())
                break;
            const auto& i = *(this->_iterator);
            if(i.isExpired())
                this->_iterator = _list.erase(this->_iterator);
            else
                break;
        } while(true);
    }

protected:
    T& _list;
};

template<typename T, typename U> class ExpirableItemConstIterator : public ExpirableItemIteratorBase<T, typename T::const_iterator> {
public:
    ExpirableItemConstIterator(T& list, typename T::const_iterator iterator)
        : ExpirableItemIteratorBase(list, iterator) {
    }

    const sp<U>& operator * () const {
        return (*this->_iterator)._item;
    }
};

template<typename T, typename U> class ExpirableItemIterator : public ExpirableItemIteratorBase<T, typename T::iterator> {
public:
    ExpirableItemIterator(T& list, typename T::iterator iterator)
        : ExpirableItemIteratorBase(list, iterator) {
    }

    sp<U>& operator * () {
        return (*this->_iterator)._item;
    }
};


template<typename T, typename V> class ListWithValidator {
private:
    typedef _ItemWithValidator<T, V> _Item;
    typedef std::list<_Item> _List;

public:
    typedef ExpirableItemConstIterator<_List, T> const_iterator;
    typedef ExpirableItemIterator<_List, T> iterator;

    template<typename... Args> void push_back(const sp<T>& item, Args&&... args) {
        _items.push_back(_ItemWithValidator<T, V>(item, V(item, std::forward<Args>(args)...)));
    }

    size_t size() const {
        return _items.size();
    }

    void clear() {
        _items.clear();
    }

    void remove(const sp<T>& item) {
        for(auto iter = _items.begin(); iter != _items.end(); ++iter) {
            const _Item& i = *iter;
            if(i._item == item) {
                _items.erase(iter);
                break;
            }
        }
    }

    const const_iterator begin() const {
        return const_iterator(_items, _items.begin());
    }

    const const_iterator end() const {
        return const_iterator(_items, _items.end());
    }

    iterator begin() {
        return iterator(_items, _items.begin());
    }

    iterator end() {
        return iterator(_items, _items.end());
    }

private:
    mutable _List _items;
};

template <typename T> using ExpirableItemList = ListWithValidator<T, typename Validator::IsExpired<T>>;
template <typename T> using WeakRefList = ListWithValidator<T, typename Validator::IsUnique<T>>;

}

#endif
