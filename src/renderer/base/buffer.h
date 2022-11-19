#ifndef ARK_RENDERER_BASE_BUFFER_H_
#define ARK_RENDERER_BASE_BUFFER_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_request.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Buffer {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_VERTEX,
        TYPE_INDEX,
        TYPE_DRAW_INDIRECT,
        TYPE_STORAGE,
        TYPE_COUNT
    };

//  [[script::bindings::enumeration]]
    enum Usage {
        USAGE_DYNAMIC,
        USAGE_STATIC,
        USAGE_COUNT
    };

    class ARK_API Delegate : public Resource {
    public:
        Delegate();
        virtual ~Delegate() = default;

        virtual void uploadBuffer(GraphicsContext& graphicsContext, Uploader& uploader) = 0;

        size_t size() const;

    protected:
        size_t _size;
    };

public:
    typedef std::pair<size_t, ByteArray::Borrowed> Strip;

    class ARK_API Snapshot {
    public:
        Snapshot() = default;
        Snapshot(sp<Delegate> stub);
        Snapshot(sp<Delegate> stub, size_t size, sp<Uploader> uploader);
        DEFAULT_COPY_AND_ASSIGN(Snapshot);

        explicit operator bool() const;

        uint64_t id() const;

        template<typename T> size_t length() const {
            return _size / sizeof(T);
        }
        size_t size() const;

        void upload(GraphicsContext& graphicsContext) const;

        const sp<Delegate>& delegate() const;

        sp<Delegate> _delegate;
        sp<Uploader> _uploader;
        size_t _size;
    };

    class ARK_API Factory {
    public:
        Factory(size_t stride);
        DEFAULT_COPY_AND_ASSIGN(Factory);

        Snapshot toSnapshot(const Buffer& buffer);

        void addStrip(size_t offset, ByteArray::Borrowed& content);

        size_t _stride;
        size_t _size;

        std::vector<Strip> _strips;
    };

public:
    Buffer(sp<Delegate> delegate) noexcept;
    Buffer() noexcept = default;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Buffer);

    explicit operator bool() const;

//  [[script::bindings::property]]
    size_t size() const;

    Snapshot snapshot(size_t size) const;
    Snapshot snapshot(const ByteArray::Borrowed& strip) const;
    Snapshot snapshot(sp<Uploader> uploader = nullptr, size_t size = 0) const;

//  [[script::bindings::property]]
    uint64_t id() const;
[[deprecated]]
//  [[script::bindings::auto]]
    void upload(sp<Uploader> uploader, sp<Future> future = nullptr);
    void upload(GraphicsContext&) const;

    const sp<Delegate>& delegate() const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Buffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Buffer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        SafePtr<Builder<Uploader>> _uploader;
        Usage _usage;

    };

private:
    sp<Delegate> _delegate;

    friend class ResourceManager;
};

}

#endif
