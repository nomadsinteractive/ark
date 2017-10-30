#ifndef ARK_CORE_CONCURRENT_DUAL_H_
#define ARK_CORE_CONCURRENT_DUAL_H_

namespace ark {

template<typename T> class Dual {
public:
    Dual()
        : _swapped(false) {
    }

    Dual(const T& h1, const T& h2)
        : _swapped(false), _h1(h1), _h2(h2) {
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

private:
    bool _swapped;
    T _h1;
    T _h2;
};

}

#endif
