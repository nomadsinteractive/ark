#ifndef ARK_CORE_INF_ARRAY_H_
#define ARK_CORE_INF_ARRAY_H_

#include <array>

#include "core/forwarding.h"

namespace ark {

template<typename T> class Array {
public:
    virtual ~Array() = default;

    virtual size_t length() = 0;
    virtual T* buf() = 0;

    size_t size() {
        return sizeof(T) * this->length();
    }

    class Allocated;
    class Borrowed;
    template<size_t LEN> class Fixed;

};


template<typename T> class Array<T>::Allocated : public Array<T> {
public:
    Allocated(size_t length)
        : _data(new T[length]), _length(length) {
    }
    Allocated(const std::initializer_list<T>& il)
        : Allocated(il.size()) {
        std::copy(il.begin(), il.end(), _data);
    }
    DISALLOW_COPY_AND_ASSIGN(Allocated);

    ~Allocated() override {
        delete[] _data;
    }

    virtual size_t length() override {
        return _length;
    }

    virtual T* buf() override {
        return _data;
    }

private:
    T* _data;
    size_t _length;
};


template<typename T> class Array<T>::Borrowed : public Array<T> {
public:
    Borrowed(T* data, size_t length)
        : _data(data), _length(length) {
    }

    virtual size_t length() override {
        return _length;
    }

    virtual T* buf() override {
        return _data;
    }

private:
    T* _data;
    size_t _length;
};


template<typename T> template<size_t LEN> class Array<T>::Fixed : public Array<T> {
public:
    Fixed() = default;
    Fixed(const std::initializer_list<T>& list) {
        std::copy(list.begin(), list.begin() + std::min(LEN, list.size()), _data.begin());
    }

    virtual size_t length() override {
        return _data.size();
    }

    virtual T* buf() override {
        return _data.data();
    }

private:
    std::array<T, LEN> _data;
};

}

#endif
