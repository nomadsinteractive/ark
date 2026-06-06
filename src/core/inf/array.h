#pragma once

#include <array>
#include <string_view>

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

template<typename T> class Array {
public:
    virtual ~Array() = default;

    virtual size_t length() = 0;
    virtual T* buf() = 0;

    size_t size() {
        return sizeof(T) * this->length();
    }

    T& at(size_t i) {
        DASSERT(i < length());
        return buf()[i];
    }

    BytesView toBytes() {
        return BytesView(static_cast<uint8_t*>(buf()), size());
    }

    class iterator {
    public:
        iterator(T* buf, size_t iter)
            : _buf(buf), _iter(iter) {
        }

        bool operator == (const iterator& other) const {
            return _buf == other._buf && _iter == other._iter;
        }

        bool operator != (const iterator& other) const {
            return _buf != other._buf || _iter != other._iter;
        }

        const iterator& operator ++() {
            ++(this->_iter);
            return *this;
        }

        iterator operator ++(int) {
            return iterator(_buf, _iter++);
        }

        T& operator *() {
            return _buf[_iter];
        }

    private:
        T* _buf;
        size_t _iter;
    };

    iterator begin() {
        return iterator(buf(), 0);
    }

    iterator end() {
        return iterator(buf(), length());
    }

    class Allocated;
    class View;
    class Vector;
    class Sliced;

    template<size_t LEN> class Fixed;
    template<typename U> class Casted;
};


template<typename T> class Array<T>::Allocated final : public Array<T> {
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

    size_t length() override {
        return _length;
    }

    T* buf() override {
        return _data;
    }

private:
    T* _data;
    size_t _length;
};


template<typename T> class Array<T>::View final : public Array<T> {
public:
    View() {
    }
    View(T* data, const size_t length)
        : _view(data, length) {
    }
    View(const std::span<T> view)
        : _view(view) {
    }
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(View);

    size_t length() override {
        return _view.size();
    }

    T* buf() override {
        return _view.data();
    }

    size_t length() const {
        return _view.size();
    }

    const T* buf() const {
        return _view.data();
    }

    T& at(const size_t i) {
        return _view[i];
    }

    const T& at(const size_t i) const {
        return _view[i];
    }

private:
    std::span<T> _view;
};


template<typename T> class Array<T>::Vector final : public Array<T> {
public:
    Vector(ark::Vector<T> data)
        : _data(std::move(data)) {
    }

    size_t length() override {
        return _data.size();
    }

    T* buf() override {
        return _data.data();
    }

private:
    ark::Vector<T> _data;
};


template<typename T> template<size_t LEN> class Array<T>::Fixed final : public Array<T> {
public:
    Fixed() = default;
    Fixed(const std::initializer_list<T>& list) {
        std::copy(list.begin(), list.begin() + std::min(LEN, list.size()), _data.begin());
    }
    DISALLOW_COPY_AND_ASSIGN(Fixed);

    size_t length() override {
        return _data.size();
    }

    T* buf() override {
        return _data.data();
    }

private:
    std::array<T, LEN> _data;
};

template<typename T> template<typename U> class Array<T>::Casted final : public Array<T> {
public:
    Casted(sp<Array<U>> data)
        : _data(std::move(data)) {
    }

    size_t length() override {
        return _data->size() / sizeof(T);
    }

    T* buf() override {
        return reinterpret_cast<T*>(_data->buf());
    }

private:
    sp<Array<U>> _data;
};

template<typename T> class Array<T>::Sliced final : public Array<T> {
public:
    Sliced(sp<Array<T>> data, const size_t offset, const size_t length)
        : _data(std::move(data)), _offset(offset), _length(length) {
    }

    size_t length() override {
        return _length;
    }

    T* buf() override {
        return _data->buf() + _offset;
    }

private:
    sp<Array<T>> _data;
    size_t _offset;
    size_t _length;
};

}
