#ifndef ARK_CORE_COLLECTION_FILTERED_LIST_H_
#define ARK_CORE_COLLECTION_FILTERED_LIST_H_

#include <list>

#include "core/forwarding.h"
#include "core/epi/lifecycle.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/shared_ptr.h"

namespace ark {

enum FilterAction {
    FILTER_ACTION_NONE,
    FILTER_ACTION_SKIP,
    FILTER_ACTION_REMOVE
};

class ListFilters {
public:

    template<typename T> class IsDisposed {
    public:
        IsDisposed(const sp<T>& item)
            : IsDisposed(item, item.template as<Lifecycle>()) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Boolean>& disposed)
            : _disposed(disposed) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Lifecycle>& lifecycle)
            : _disposed(lifecycle ? lifecycle->toBoolean() : sp<Boolean>::null()) {
        }

        FilterAction operator()(const sp<T>& /*item*/) const {
            return _disposed && _disposed->val() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
        }

    private:
        sp<Boolean> _disposed;
    };

    template<typename T> class IsUnique {
    public:
        IsUnique(const sp<T>& /*item*/) {
        }

        FilterAction operator()(const sp<T>& item) const {
            return item.unique() ? FILTER_ACTION_REMOVE : FILTER_ACTION_NONE;
        }

    };

};

template<typename T, typename Filter> class FilteredList {
private:
    template<typename T> struct ItemWithFilter {
        ItemWithFilter(sp<T> item, Filter filter)
            : _item(std::move(item)), _filter(std::move(filter)) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(ItemWithFilter);

        bool operator < (const ItemWithFilter& other) const {
            return _item < other._item;
        }

        sp<T> _item;
        Filter _filter;
    };

private:
    typedef ItemWithFilter<T> _Item;
    typedef std::list<_Item> _List;

public:
    template<typename U> class Iterator : public IteratorBase<U> {
    public:
        Iterator(_List& list, U iterator)
            : IteratorBase<U>(iterator), _list(list) {
            forward();
        }

        const Iterator<U>& operator ++() {
            ++(this->_iterator);
            forward();
            return *this;
        }

        const Iterator<U> operator ++(int) {
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
                FilterAction fa = i._filter(i._item);
                if(fa == FILTER_ACTION_NONE)
                    break;
                if(fa == FILTER_ACTION_REMOVE)
                    this->_iterator = _list.erase(this->_iterator);
            } while(true);
        }

    private:
        _List& _list;
    };

public:
    typedef Iterator<typename _List::iterator> iterator;

    template<typename... Args> void push_back(const sp<T>& item, Args&&... args) {
        _items.push_back(ItemWithFilter<T>(item, Filter(item, std::forward<Args>(args)...)));
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


template <typename T> using ListWithLifecycle = FilteredList<T, typename ListFilters::IsDisposed<T>>;
template <typename T> using WeakRefList = FilteredList<T, typename ListFilters::IsUnique<T>>;

}

#endif
