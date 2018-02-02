#ifndef ARK_CORE_CONCURRENT_DUAL_H_
#define ARK_CORE_CONCURRENT_DUAL_H_

#include <atomic>

namespace ark {

template<typename T> class Dual {
public:
    Dual()
        : _swapped(false) {
    }
    Dual(const T& front)
        : _swapped(false), _h2(front) {
    }
    Dual(const T& h1, const T& h2)
        : _swapped(false), _h1(h1), _h2(h2) {
    }

    operator const T&() const {
        return front();
    }

    const T* operator ->() const {
        return _swapped ? &_h1 : &_h2;
    }

    const T& front() const {
        return _swapped ? _h1 : _h2;
    }

    T& back() {
        return _swapped ? _h2 : _h1;
    }

    void swap() {
        _swapped = !_swapped;
    }

    void setAndSwap(const T& value) {
        back() = value;
        swap();
    }

private:
    std::atomic<bool> _swapped;
    T _h1;
    T _h2;
};

}

#endif
