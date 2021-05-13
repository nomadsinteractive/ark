#ifndef ARK_CORE_COLLECTION_LIST_H_
#define ARK_CORE_COLLECTION_LIST_H_

#include <list>
#include <stdint.h>

#include "core/forwarding.h"
#include "core/epi/disposed.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/safe_var.h"

namespace ark {

enum FilterAction {
    FILTER_ACTION_NONE,
    FILTER_ACTION_REMOVE
};

class ListFilters {
public:
    template<typename T> class IsDisposed {
    public:
        IsDisposed(const sp<T>& item)
            : IsDisposed(item, item.template as<Disposed>()) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Boolean>& disposed)
            : _disposed(disposed, false) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Disposed>& disposed)
            : _disposed(disposed, false) {
        }

        FilterAction operator() (uint64_t timestamp) const {
            _disposed.update(timestamp);
            return _disposed.val() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
        }

    private:
        SafeVar<Boolean> _disposed;
    };
};

template<typename T, typename Filter> class List {
private:
    struct Item {
        Item(sp<T> item, Filter filter)
            : _item(std::move(item)), _filter(std::move(filter)) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        sp<T> _item;
        Filter _filter;
    };

private:
    typedef std::list<Item> _List;

public:
    template<typename U> class FilteredIterator : public IteratorBase<U> {
    public:
        FilteredIterator(_List& list, U iterator)
            : IteratorBase<U>(std::move(iterator)), _list(list) {
            forward();
        }

        const FilteredIterator<U>& operator ++() {
            DASSERT(this->_iterator != _list.end());
            ++(this->_iterator);
            forward();
            return *this;
        }

        const FilteredIterator<U> operator ++(int) {
            DASSERT(this->_iterator != _list.end());
            U iter = this->_iterator;
            ++(this->_iterator);
            forward();
            return iter;
        }

        sp<T>& operator *() {
            return (*this->_iterator)._item;
        }

    private:
        void forward() {
            do {
                if(this->_iterator == _list.end())
                    break;
                const auto& i = *(this->_iterator);
                _filter_action = i._filter();
                if(_filter_action == FILTER_ACTION_NONE)
                    break;
                if(_filter_action == FILTER_ACTION_REMOVE) {
                    this->_iterator = _list.erase(this->_iterator);
                }
            } while(true);
        }

    private:
        _List& _list;
        FilterAction _filter_action;
    };

    template<typename U> class Iterator : public IteratorBase<U> {
    public:
        Iterator(_List& list, U iterator)
            : IteratorBase<U>(std::move(iterator)), _list(list) {
        }

        const Iterator<U>& operator ++() {
            DASSERT(this->_iterator != _list.end());
            ++(this->_iterator);
            return *this;
        }

        const Iterator<U> operator ++(int) {
            DASSERT(this->_iterator != _list.end());
            U iter = this->_iterator;
            ++(this->_iterator);
            return iter;
        }

        sp<T>& operator *() {
            return (*this->_iterator)._item;
        }

    private:
        _List& _list;
    };

    typedef FilteredIterator<typename _List::iterator> filtered_iterator;
    typedef Iterator<typename _List::iterator> iterator;

    class UpdatedList {
    public:
        UpdatedList(_List& items)
            : _items(items) {
        }

        iterator begin() {
            return iterator(_items, _items.begin());
        }

        iterator end() {
            return iterator(_items, _items.end());
        }

    private:
        _List& _items;
    };

    template<typename... Args> void push_back(const sp<T>& item, Args&&... args) {
        _items.emplace_back(item, Filter(item, std::forward<Args>(args)...));
    }

    UpdatedList update(uint64_t timestamp) {
        for(auto iter = _items.begin(); iter != _items.end(); ++iter) {
            const auto& i = *(iter);
            const FilterAction fa = i._filter(timestamp);
            if(fa == FILTER_ACTION_REMOVE) {
                iter = _items.erase(iter);
                if(iter == _items.end())
                    break;
            }
        }
        return UpdatedList(_items);
    }

    const _List& items() const {
        return _items;
    }

    filtered_iterator begin() {
        return filtered_iterator(_items, _items.begin());
    }

    filtered_iterator end() {
        return filtered_iterator(_items, _items.end());
    }

private:
    _List _items;
};


template <typename T> using DisposableItemList = List<T, typename ListFilters::IsDisposed<T>>;

}

#endif
