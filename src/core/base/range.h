#ifndef ARK_CORE_BASE_RANGE_H_
#define ARK_CORE_BASE_RANGE_H_

#include "core/forwarding.h"
#include "core/collection/iterable.h"

namespace ark {

template<typename T> class Range {
public:
    class iterator : public IteratorBase<T*> {
    public:
        iterator(T* ptr, T* end, ptrdiff_t step)
            : IteratorBase(ptr), _end(end), _step(step) {
        }

        iterator& operator ++() {
            this->_iterator += _step;
            DASSERT((this->_iterator <= _end && _step > 0) || (this->_iterator >= _end && _step < 0));
            return *this;
        }

        iterator operator ++(int) {
            ++(*this);
            return iterator(this->_iterator - _step, _step);
        }

        const T& operator *() const {
            return *this->_iterator;
        }

        T& operator *() {
            return *this->_iterator;
        }

    private:
        T* _end;
        ptrdiff_t _step;
    };

    class const_iterator : public IteratorBase<const T*>{
    public:
        const_iterator(const T* ptr, const T* end, ptrdiff_t step)
            : IteratorBase(ptr), _end(end), _step(step) {
        }

        iterator& operator ++() {
            this->_iterator += _step;
            DASSERT((this->_iterator <= _end && _step > 0) || (this->_iterator >= _end && _step < 0));
            return *this;
        }

        iterator operator ++(int) {
            ++(*this);
            return iterator(this->_iterator - _step, _step);
        }

        const T& operator *() const {
            return *this->_iterator;
        }

    private:
        const T* _end;
        ptrdiff_t _step;

    };

public:
    Range(T* begin, T* end, ptrdiff_t step = 1)
        : _begin(begin), _end(end), _step(step) {
        DCHECK(((_end >= _begin && step > 0) || (_end <= _begin && step < 0)) && std::abs(_end - _begin) % std::abs(_step) == 0, "Illegal Range(%p, %p, %zd)", _begin, _end, _step);
    }

    iterator begin() {
        return iterator(_begin, _end, _step);
    }

    iterator end() {
        return iterator(_end, _end, _step);
    }

    const_iterator begin() const {
        return const_iterator(_begin, _end, _step);
    }

    const_iterator end() const {
        return const_iterator(_end, _end, _step);
    }

private:
    T* _begin;
    T* _end;
    ptrdiff_t _step;

};

}

#endif
