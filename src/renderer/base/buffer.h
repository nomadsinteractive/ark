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

    class ARK_API Delegate : public Resource {
    public:
        Delegate();
        virtual ~Delegate() = default;

        size_t size() const;

    protected:
        size_t _size;
    };

public:
    class ARK_API Snapshot {
    public:
        Snapshot() = default;
        Snapshot(const sp<Delegate>& stub);
        Snapshot(const sp<Delegate>& stub, size_t size);
        Snapshot(const sp<Delegate>& stub, const sp<Uploader>& uploader);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        explicit operator bool() const;

        uint64_t id() const;

        template<typename T> size_t length() const {
            return _size / sizeof(T);
        }
        size_t size() const;

        void upload(GraphicsContext& graphicsContext) const;

        const sp<Delegate>& delegate() const;

    private:
        sp<Delegate> _delegate;
        sp<Uploader> _uploader;
        size_t _size;
    };

    class ARK_API Builder {
    public:
        Builder(size_t stride, size_t growCapacity);
        DEFAULT_COPY_AND_ASSIGN(Builder);

        template<typename T> void write(const T& value, size_t offset = 0) {
            DCHECK(sizeof(T) + offset <= _stride, "Stride overflow: sizeof(value) = %d, offset = %d", sizeof(value), offset);
            memcpy(_ptr + offset, &value, sizeof(T));
        }

        template<typename T> void write(const T& value, const int32_t* offsets, int32_t name) {
            if(offsets[name] >= 0)
                write<T>(value, offsets[name]);
        }

        void writeArray(ByteArray& array);

        void setGrowCapacity(size_t growCapacity);
        void next();

        Snapshot toSnapshot(const Buffer& buffer) const;

        size_t stride() const;
        size_t length() const;

    private:
        void grow();

        sp<Uploader> makeUploader() const;

    private:
        size_t _stride;
        size_t _grow_capacity;

        uint8_t* _ptr;
        uint8_t* _boundary;
        std::vector<bytearray> _buffers;

        size_t _size;
    };

public:
    Buffer(const sp<Delegate>& delegate) noexcept;
    Buffer() noexcept = default;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Buffer);

    explicit operator bool() const;

    size_t size() const;

    Snapshot snapshot(const sp<Uploader>& uploader) const;
    Snapshot snapshot(size_t size) const;
    Snapshot snapshot() const;

    uint64_t id() const;
    void upload(GraphicsContext&) const;

    const sp<Delegate>& delegate() const;

private:
    sp<Delegate> _delegate;

    friend class ResourceManager;
    friend class RenderController;
};

}

#endif
