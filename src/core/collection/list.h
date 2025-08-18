#pragma once

#include <list>
#include <stdint.h>

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/collection/iterable.h"
#include "core/types/optional_var.h"
#include "core/components/visibility.h"

namespace ark {

enum FilterAction {
    FILTER_ACTION_NONE,
    FILTER_ACTION_SKIP,
    FILTER_ACTION_REMOVE
};

class ListFilters {
public:
    template<typename T, FilterAction FA1 = FILTER_ACTION_REMOVE, FilterAction FA2 = FILTER_ACTION_NONE> class Unique {
    public:
        Unique() = default;

        FilterAction operator() (const T& item) const {
            return item.unique() ? FA1 : FA2;
        }
    };

    template<typename T, FilterAction FA1, FilterAction FA2, bool DF> class IsTrue {
    public:
        IsTrue(sp<Boolean> filter)
            : _condition(std::move(filter), DF) {
        }

        void update(const uint64_t timestamp) const {
            _condition.update(timestamp);
        }

        FilterAction operator() (const T& /*item*/) const {
            return _condition.val() ? FA1 : FA2;
        }

    private:
        OptionalVar<Boolean> _condition;
    };

    template<typename T, typename U, typename V> class IsBoth {
    public:
        IsBoth(sp<Boolean> s1, sp<Boolean> s2)
            : _op1(std::move(s1)), _op2(std::move(s2)) {
        }

        void update(uint64_t timestamp) const {
            _op1.update(timestamp);
            _op2.update(timestamp);
        }

        FilterAction operator() (const T& item) const {
            FilterAction fa1 = _op1(item);
            return fa1 == FILTER_ACTION_NONE ? _op2(item) : fa1;
        }

    private:
        U _op1;
        V _op2;
    };

    template <typename T> using IsDiscarded = IsTrue<T, FILTER_ACTION_REMOVE, FILTER_ACTION_NONE, false>;
    template <typename T> using IsVisible = IsTrue<T, FILTER_ACTION_NONE, FILTER_ACTION_SKIP, true>;
    template <typename T> using IsDV = IsBoth<T, IsDiscarded<T>, IsVisible<T>>;
};

template<typename T, typename Filter> class FList {
private:
    struct Item {
        Item(T item, Filter filter)
            : _item(std::move(item)), _filter(std::move(filter)) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Item);

        T _item;
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

        FilteredIterator<U> operator ++(int) {
            DASSERT(this->_iterator != _list.end());
            U iter = this->_iterator;
            ++(this->_iterator);
            forward();
            return iter;
        }

        const T& operator *() const {
            return this->_iterator->_item;
        }

        T& operator *() {
            return this->_iterator->_item;
        }

    private:
        void forward() {
            do {
                if(this->_iterator == _list.end())
                    break;
                auto& i = *(this->_iterator);
                _call_update_sfinae(i._filter, nullptr);

                if(const FilterAction fa = i._filter(i._item); fa == FILTER_ACTION_REMOVE)
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
        UpdatedList(ListImpl& items, const uint64_t timestamp)
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

    template<typename... Args> void emplace_front(T item, Args&&... args) {
        Filter filter(std::forward<Args>(args)...);
        _items.emplace_front(std::move(item), std::move(filter));
    }

    template<typename... Args> void emplace_back(T item, Args&&... args) {
        Filter filter(std::forward<Args>(args)...);
        _items.emplace_back(std::move(item), std::move(filter));
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

template <typename T> using U_FList = FList<T, typename ListFilters::Unique<T>>;
template <typename T> using D_FList = FList<T, typename ListFilters::IsDiscarded<T>>;
template <typename T> using DV_FList = FList<T, typename ListFilters::IsDV<T>>;

}
