#ifndef ARK_RENDERER_BASE_BUFFER_H_
#define ARK_RENDERER_BASE_BUFFER_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Buffer {
public:
    typedef std::function<void(void*, size_t)> UploadFunc;

    enum Type {
        TYPE_VERTEX,
        TYPE_INDEX,
        TYPE_COUNT
    };

    enum Usage {
        USAGE_DYNAMIC,
        USAGE_STATIC,
        USAGE_COUNT
    };

    enum Name {
        NAME_NONE = -1,
        NAME_QUADS,
        NAME_NINE_PATCH,
        NAME_POINTS,
        NAME_COUNT
    };

    class ARK_API Delegate : public Resource {
    public:
        Delegate(size_t size);
        virtual ~Delegate() = default;

        size_t size() const;

        virtual void reload(GraphicsContext& graphicsContext, const sp<Uploader>& transientUploader) = 0;

    protected:
        size_t _size;
    };

public:
    class Snapshot {
    public:
        Snapshot() = default;
        Snapshot(const sp<Delegate>& stub);
        Snapshot(const sp<Delegate>& stub, size_t size);
        Snapshot(const sp<Delegate>& stub, const sp<Uploader>& uploader);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        uint32_t id() const;

        template<typename T> size_t length() const {
            return _size / sizeof(T);
        }
        size_t size() const;

        void upload(GraphicsContext& graphicsContext) const;

    private:
        sp<Delegate> _delegate;
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
    Buffer(const sp<Delegate>& delegate);
    Buffer() noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Buffer);

    explicit operator bool() const;

    size_t size() const;

    Snapshot snapshot(const sp<Uploader>& uploader) const;
    Snapshot snapshot(size_t size) const;
    Snapshot snapshot() const;

    uint32_t id() const;
    void upload(GraphicsContext&) const;

private:
    sp<Delegate> _delegate;

    friend class ResourceManager;
};

}

#endif