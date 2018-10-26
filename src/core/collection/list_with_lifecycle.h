#ifndef ARK_CORE_COLLECTION_LIST_WITH_LIFECYCLE_H_
#define ARK_CORE_COLLECTION_LIST_WITH_LIFECYCLE_H_

#include <list>

#include "core/forwarding.h"
#include "core/epi/lifecycle.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Validator {
public:

    template<typename T> class IsDisposed {
    public:
        IsDisposed(const sp<T>& item)
            : _disposed(item.template as<Lifecycle>()) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Boolean>& disposed)
            : _disposed(disposed) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(IsDisposed);

        bool operator()(const sp<T>& /*item*/) const {
            return _disposed && _disposed->val();
        }

    private:
        sp<Boolean> _disposed;
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
    _ItemWithValidator(sp<T> item, V validator)
        : _item(std::move(item)), _validator(std::move(validator)) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(_ItemWithValidator);

    bool isDisposed() const {
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
            if(i.isDisposed())
                this->_iterator = _list.erase(this->_iterator);
            else
                break;
        } while(true);
    }

protected:
    T& _list;
};

template<typename T, typename U> class ExpirableItemIterator : public ExpirableItemIteratorBase<T, typename T::iterator> {
public:
    ExpirableItemIterator(T& list, typename T::iterator iterator)
        : ExpirableItemIteratorBase<T, typename T::iterator>(list, iterator) {
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

    iterator begin() {
        return iterator(_items, _items.begin());
    }

    iterator end() {
        return iterator(_items, _items.end());
    }

private:
    _List _items;
};

template <typename T> using ListWithLifecycle = ListWithValidator<T, typename Validator::IsDisposed<T>>;
template <typename T> using WeakRefList = ListWithValidator<T, typename Validator::IsUnique<T>>;

}

#endif
