#pragma once

namespace ark {

template<typename T> class IteratorBase {
public:
    IteratorBase(T iterator)
        : _iterator(std::move(iterator)) {
    }

    bool operator == (const IteratorBase<T>& other) const {
        return _iterator == other._iterator;
    }

    bool operator != (const IteratorBase<T>& other) const {
        return _iterator != other._iterator;
    }

protected:
    T _iterator;
};

}
