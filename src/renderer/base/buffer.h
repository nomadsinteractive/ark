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
    enum Type {
        TYPE_VERTEX,
        TYPE_INDEX,
        TYPE_DRAW_INDIRECT,
        TYPE_STORAGE,
        TYPE_COUNT
    };

    enum Usage {
        USAGE_DYNAMIC,
        USAGE_STATIC,
        USAGE_COUNT
    };

    class ARK_API Snapshot;

    class ARK_API Delegate : public Resource {
    public:
        Delegate();
        virtual ~Delegate() = default;

        size_t size() const;

        void setUploader(sp<Uploader> uploader);
//        virtual void upload(GraphicsContext& graphicsContext, const Snapshot& snapshot) = 0;

    protected:
        size_t _size;
        sp<Uploader> _uploader;
    };

public:
    typedef std::pair<size_t, ByteArray::Borrowed> Strip;

    class ARK_API Snapshot {
    public:
        Snapshot() = default;
        Snapshot(sp<Delegate> stub);
        Snapshot(sp<Delegate> stub, size_t size);
        Snapshot(sp<Delegate> stub, sp<Uploader> uploader);
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

[[deprecated]]
    Snapshot snapshot(const sp<Uploader>& uploader) const;
    Snapshot snapshot(const ByteArray::Borrowed& strip) const;
    Snapshot snapshot(size_t size) const;
    Snapshot snapshot() const;

//  [[script::bindings::property]]
    uint64_t id() const;
    void upload(GraphicsContext&) const;

    const sp<Delegate>& delegate() const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Buffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Buffer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Input>> _input;
        sp<Builder<Integer>> _length;
        SafePtr<Builder<Integer>> _stride;
        Usage _usage;

        std::vector<sp<Builder<Input>>> _vars;
    };

private:
    sp<Delegate> _delegate;

    friend class ResourceManager;
    friend class RenderController;
};

}

#endif
