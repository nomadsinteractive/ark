#ifndef ARK_RENDERER_INF_UPLOADER_H_
#define ARK_RENDERER_INF_UPLOADER_H_

#include <functional>
#include <vector>

#include "core/base/api.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Uploader {
public:
    typedef std::function<void(void*, size_t)>  UploadFunc;
    typedef std::function<sp<Uploader>(size_t)> MakerFunc;

    Uploader(size_t size);
    virtual ~Uploader() = default;

    size_t size() const;
    virtual void upload(const UploadFunc& uploader) = 0;

    template<typename T> class Array;
    template<typename T> class ArrayList;
    template<typename T> class Vector;
    template<typename T> class StandardLayout;

protected:
    size_t _size;
};

template<typename T> class Uploader::Array : public Uploader {
public:
    Array(const sp<ark::Array<T>>& array)
        : Uploader(array->size()), _array(array) {
    }

    virtual void upload(const UploadFunc& uploader) override {
        uploader(_array->buf(), _array->size());
    }

private:
    sp<ark::Array<T>> _array;
};

template<typename T> class Uploader::ArrayList : public Uploader {
public:
    ArrayList(std::vector<sp<ark::Array<T>>> arrayList)
        : Uploader(0), _array_list(std::move(arrayList)) {
        for(const auto& i : _array_list)
            _size += i->length();
    }

    virtual void upload(const UploadFunc& uploader) override {
        for(const auto& i : _array_list)
            uploader(i->buf(), i->size());
    }

private:
    std::vector<bytearray> _array_list;
};

template<typename T> class Uploader::Vector : public Uploader {
public:
    Vector(std::vector<T> vector)
        : Uploader(vector.size() * sizeof(T)), _vector(std::move(vector)) {
    }

    virtual void upload(const UploadFunc& uploader) override {
        uploader(&_vector[0], _vector.size() * sizeof(T));
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

    virtual void upload(const UploadFunc& uploader) override {
        uploader(_object.get(), _size);
    }

private:
    sp<T> _object;
};

typedef Uploader::Array<uint8_t> ByteArrayUploader;
typedef Uploader::ArrayList<uint8_t> ByteArrayListUploader;

typedef Uploader::Array<glindex_t> IndexArrayUploader;
typedef Uploader::ArrayList<glindex_t> IndexArrayListUploader;

}

#endif
