#ifndef ARK_CORE_COLLECTION_LIST_H_
#define ARK_CORE_COLLECTION_LIST_H_

#include <list>

#include "core/forwarding.h"
#include "core/epi/disposed.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/shared_ptr.h"

namespace ark {

enum FilterAction {
    FILTER_ACTION_NONE,
    FILTER_ACTION_SKIP,
    FILTER_ACTION_REMOVE,
    FILTER_ACTION_REMOVE_AFTER
};

class ListFilters {
public:

    template<typename T> class IsDisposed {
    public:
        IsDisposed(const sp<T>& item)
            : IsDisposed(item, item.template as<Disposed>()) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Boolean>& disposed)
            : _disposed(disposed) {
        }
        IsDisposed(const sp<T>& /*item*/, const sp<Disposed>& disposed)
            : _disposed(disposed) {
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
            return _filter_action == FILTER_ACTION_REMOVE_AFTER ? _removed_after : (*this->_iterator)._item;
        }

    private:
        void forward() {
            do {
                if(_filter_action == FILTER_ACTION_REMOVE_AFTER)
                    _removed_after = nullptr;

                if(this->_iterator == _list.end())
                    break;
                const auto& i = *(this->_iterator);
                _filter_action = i._filter(i._item);
                if(_filter_action == FILTER_ACTION_NONE)
                    break;
                if(_filter_action == FILTER_ACTION_REMOVE || _filter_action == FILTER_ACTION_REMOVE_AFTER) {
                    if(_filter_action == FILTER_ACTION_REMOVE_AFTER)
                        _removed_after = (*this->_iterator)._item;
                    this->_iterator = _list.erase(this->_iterator);
                }
            } while(true);
        }

    private:
        _List& _list;
        FilterAction _filter_action;
        sp<T> _removed_after;
    };

public:
    typedef Iterator<typename _List::iterator> iterator;

    template<typename... Args> void push_back(const sp<T>& item, Args&&... args) {
        _items.emplace_back(item, Filter(item, std::forward<Args>(args)...));
    }

    const _List& items() const {
        return _items;
    }

    void clear() {
        _items.clear();
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


template <typename T> using DisposableItemList = List<T, typename ListFilters::IsDisposed<T>>;
template <typename T> using WeakRefList = List<T, typename ListFilters::IsUnique<T>>;

}

#endif
