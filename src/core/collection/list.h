#ifndef ARK_CORE_COLLECTION_LIST_H_
#define ARK_CORE_COLLECTION_LIST_H_

#include <list>
#include <stdint.h>

#include "core/forwarding.h"
#include "core/epi/disposed.h"
#include "core/epi/visibility.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/safe_var.h"

namespace ark {

enum FilterAction {
    FILTER_ACTION_NONE,
    FILTER_ACTION_SKIP,
    FILTER_ACTION_REMOVE
};

class ListFilters {
public:
    template<typename T, typename U, FilterAction FA1, FilterAction FA2, bool DF> class FilterBoolean {
    public:
        FilterBoolean(const sp<T>& item)
            : FilterBoolean(item, item.template as<U>()) {
        }
        FilterBoolean(const sp<T>& /*item*/, sp<Boolean> filter)
            : _filter(std::move(filter), DF) {
        }

        void update(uint64_t timestamp) const {
            _filter.update(timestamp);
        }

        FilterAction operator() () const {
            return _filter.val() ? FA1 : FA2;
        }

    private:
        SafeVar<Boolean> _filter;
    };

    template<typename T, typename U, typename V> class IsBoth {
    public:
        IsBoth(const sp<T>& item)
            : _op1(item), _op2(item) {
        }
        IsBoth(sp<T> item, sp<Boolean> s1, sp<Boolean> s2)
            : _op1(item, std::move(s1)), _op2(item, std::move(s2)) {
        }

        void update(uint64_t timestamp) const {
            _op1.update(timestamp);
            _op2.update(timestamp);
        }

        FilterAction operator() () const {
            FilterAction fa1 = _op1();
            return fa1 == FILTER_ACTION_NONE ? _op2() : fa1;
        }

    private:
        U _op1;
        V _op2;
    };

    template <typename T> using IsDisposable = FilterBoolean<T, Disposed, FILTER_ACTION_REMOVE, FILTER_ACTION_NONE, false>;
    template <typename T> using IsVisible = FilterBoolean<T, Visibility, FILTER_ACTION_NONE, FILTER_ACTION_SKIP, true>;
    template <typename T> using IsDV = IsBoth<T, IsDisposable<T>, IsVisible<T>>;
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

    typedef std::list<Item> ListImpl;

public:
    template<typename U> class FilteredIterator : public IteratorBase<U> {
    public:
        FilteredIterator(ListImpl& list, U iterator, uint64_t timestamp = 0)
            : IteratorBase<U>(std::move(iterator)), _list(list), _timestamp(timestamp) {
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
                _call_update_sfinae(i._filter, nullptr);
                FilterAction fa = i._filter();
                if(fa == FILTER_ACTION_REMOVE)
                    this->_iterator = _list.erase(this->_iterator);
                else if(fa == FILTER_ACTION_SKIP)
                    ++(this->_iterator);
                else
                    break;
            } while(true);
        }

        template<typename V> void _call_update_sfinae(const V& filter, decltype(filter.update(static_cast<uint64_t>(0)))*) {
            filter.update(_timestamp);
        }
        template<typename V> void _call_update_sfinae(const V& /*filter*/, ...) {
        }

    private:
        ListImpl& _list;
        uint64_t _timestamp;
    };

    typedef FilteredIterator<typename ListImpl::iterator> filtered_iterator;

    class UpdatedList {
    public:
        UpdatedList(ListImpl& items, uint64_t timestamp)
            : _items(items), _timestamp(timestamp) {
        }

        filtered_iterator begin() {
            return filtered_iterator(_items, _items.begin(), _timestamp);
        }

        filtered_iterator end() {
            return filtered_iterator(_items, _items.end(), _timestamp);
        }

    private:
        ListImpl& _items;
        uint64_t _timestamp;
    };

    template<typename... Args> void push_back(const sp<T>& item, Args&&... args) {
        _items.emplace_back(item, Filter(item, std::forward<Args>(args)...));
    }

    UpdatedList update(uint64_t timestamp) {
        return UpdatedList(_items, timestamp);
    }

    const ListImpl& items() const {
        return _items;
    }

    filtered_iterator begin() {
        return filtered_iterator(_items, _items.begin());
    }

    filtered_iterator end() {
        return filtered_iterator(_items, _items.end());
    }

private:
    ListImpl _items;
};


template <typename T> using DList = List<T, typename ListFilters::IsDisposable<T>>;
template <typename T> using DVList = List<T, typename ListFilters::IsDV<T>>;

}

#endif
