#ifndef ARK_RENDERER_INF_UPLOADER_H_
#define ARK_RENDERER_INF_UPLOADER_H_

#include <functional>
#include <string.h>
#include <vector>

#include "core/base/api.h"
#include "core/inf/array.h"
#include "core/inf/writable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Uploader {
public:
    typedef std::function<sp<Uploader>(size_t)> MakerFunc;

    Uploader(size_t size);
    virtual ~Uploader() = default;

    virtual size_t size();
    virtual void upload(Writable& writable) = 0;

    template<typename T> class Array;
    template<typename T> class Vector;
    template<typename T> class StandardLayout;

    class Blank;

protected:
    size_t _size;
};

template<typename T> class Uploader::Array : public Uploader {
public:
    Array(sp<ark::Array<T>> array)
        : Uploader(array->size()), _array(std::move(array)) {
    }

    virtual void upload(Writable& uploader) override {
        uploader.write(_array->buf(), static_cast<uint32_t>(_array->size()), 0);
    }

private:
    sp<ark::Array<T>> _array;
};

template<typename T> class Uploader::Vector : public Uploader {
public:
    Vector(std::vector<T> vector)
        : Uploader(vector.size() * sizeof(T)), _vector(std::move(vector)) {
    }

    virtual void upload(Writable& uploader) override {
        uploader.write(&_vector[0], _vector.size() * sizeof(T), 0);
    }

private:
    std::vector<T> _vector;
};

template<typename T> class Uploader::StandardLayout : public Uploader {
public:
    StandardLayout(sp<T> object)
        : Uploader(sizeof(T)), _object(std::move(object)) {
        DWARN(std::is_standard_layout<T>::value, "T is not a StandardLayoutType");
    }

    virtual void upload(Writable& uploader) override {
        uploader.write(_object.get(), _size, 0);
    }

private:
    sp<T> _object;
};

class Uploader::Blank : public Uploader {
public:
    Blank(size_t size)
        : Uploader(size), _memory(new uint8_t[size]) {
        memset(_memory, 0, _size);
    }
    ~Blank() override {
        delete[] _memory;
    }

    virtual void upload(Writable& uploader) override {
        uploader.write(_memory, static_cast<uint32_t>(_size), 0);
    }

private:
    uint8_t* _memory;
};

typedef Uploader::Array<uint8_t> ByteArrayUploader;
typedef Uploader::Array<element_index_t> IndexArrayUploader;

}

#endif
