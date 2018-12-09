#ifndef ARK_RENDERER_BASE_BUFFER_H_
#define ARK_RENDERER_BASE_BUFFER_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API Buffer {
public:
    typedef std::function<void(void*, size_t)> UploadFunc;

    class ARK_API Uploader {
    public:
        Uploader(size_t size);
        virtual ~Uploader() = default;

        size_t size() const;
        virtual void upload(const UploadFunc& uploader) = 0;

    protected:
        size_t _size;
    };

    typedef std::function<sp<Uploader>(size_t)> UploadMakerFunc;

    template<typename T> class ArrayUploader : public Uploader {
    public:
        ArrayUploader(const sp<Array<T>>& array)
            : Uploader(array->size()), _array(array) {
        }

        virtual void upload(const UploadFunc& uploader) override {
            uploader(_array->buf(), _array->size());
        }

    private:
        sp<Array<T>> _array;
    };

    template<typename T> class ArrayListUploader : public Uploader {
    public:
        ArrayListUploader(std::vector<sp<Array<T>>> arrayList)
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

    template<typename T> class VectorUploader : public Uploader {
    public:
        VectorUploader(std::vector<T> vector)
            : Uploader(vector.size() * sizeof(T)), _vector(std::move(vector)) {
        }

        virtual void upload(const UploadFunc& uploader) override {
            uploader(&_vector[0], _vector.size() * sizeof(T));
        }

    private:
        std::vector<T> _vector;
    };

    typedef ArrayUploader<uint8_t> ByteArrayUploader;
    typedef ArrayListUploader<uint8_t> ByteArrayListUploader;

    typedef ArrayUploader<glindex_t> IndexArrayUploader;
    typedef ArrayListUploader<glindex_t> IndexArrayListUploader;

private:
    class Stub : public RenderResource {
    public:
        Stub(const sp<GLRecycler>& recycler, const sp<Uploader>& uploader, GLenum type, GLenum usage);
        ~Stub() override;

        size_t size() const;

        virtual GLuint id() override;
        virtual void upload(GraphicsContext&) override;
        virtual Recycler recycle() override;

        void upload(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader);

    private:
        void doUpload(GraphicsContext& graphicsContext, Uploader& uploader);

    private:
        sp<GLRecycler> _recycler;
        sp<Uploader> _uploader;

        GLenum _type;
        GLenum _usage;

        GLuint _id;
        size_t _size;
    };

public:
    enum Name {
        NAME_NONE = -1,
        NAME_QUADS,
        NAME_NINE_PATCH,
        NAME_POINTS,
        NAME_COUNT
    };

    class Snapshot {
    public:
        Snapshot() = default;
        Snapshot(const sp<Stub>& stub);
        Snapshot(const sp<Stub>& stub, size_t size);
        Snapshot(const sp<Stub>& stub, const sp<Uploader>& uploader);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        uint32_t id() const;

        template<typename T> size_t length() const {
            return _size / sizeof(T);
        }
        size_t size() const;

        void prepare(GraphicsContext& graphicsContext) const;

    private:
        sp<Stub> _stub;
        sp<Uploader> _uploader;
        size_t _size;
    };

    class Builder {
    public:
        Builder(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t stride, size_t growCapacity);
        DEFAULT_COPY_AND_ASSIGN(Builder);

        template<typename T> void write(const T& value, size_t offset = 0) {
            DCHECK(sizeof(T) + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", sizeof(value), offset);
            memcpy(_ptr + offset, &value, sizeof(T));
        }

        template<typename T> void write(const T& value, const int32_t* offsets, int32_t name) {
            if(offsets[name] >= 0)
                write<T>(value, offsets[name]);
        }

        void setGrowCapacity(size_t growCapacity);

        void apply(const bytearray& buf);
        void next();

        sp<Uploader> makeUploader() const;

        size_t stride() const;
        size_t size() const;

    private:
        void grow();

    private:
        sp<MemoryPool> _memory_pool;
        sp<ObjectPool> _object_pool;

        size_t _stride;
        size_t _grow_capacity;

        uint8_t* _ptr;
        uint8_t* _boundary;
        std::vector<bytearray> _buffers;

        size_t _size;
    };

public:
    Buffer(const sp<GLRecycler>& recycler, const sp<Buffer::Uploader>& uploader, GLenum type, GLenum usage) noexcept;
    Buffer() noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Buffer);

    explicit operator bool() const;

    size_t size() const;

    Snapshot snapshot(const sp<Uploader>& uploader) const;
    Snapshot snapshot(size_t size) const;
    Snapshot snapshot() const;

    GLuint id() const;
    void upload(GraphicsContext&) const;

private:
    sp<Stub> _stub;

    friend class GLResourceManager;
};

}

#endif
