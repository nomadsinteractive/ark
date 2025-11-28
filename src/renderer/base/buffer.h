#pragma once

#include "core/base/api.h"
#include "core/base/bit_set.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"
#include "core/impl/builder/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/render_request.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Buffer {
public:
//  [[script::bindings::enumeration]]
    enum UsageBits {
        USAGE_BIT_VERTEX,
        USAGE_BIT_INDEX,
        USAGE_BIT_DRAW_INDIRECT,
        USAGE_BIT_STORAGE,
        USAGE_BIT_DYNAMIC,
        USAGE_BIT_TRANSFER_SRC,
        USAGE_BIT_HOST_VISIBLE
    };
    typedef BitSet<UsageBits, true> Usage;

    class ARK_API Delegate : public Resource {
    public:
        Delegate() = default;

        virtual void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) = 0;
        virtual void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) = 0;

        size_t size() const;
        void setSize(size_t size);

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

    class ARK_API SnapshotFactory {
    public:
        SnapshotFactory(size_t stride);
        DEFAULT_COPY_AND_ASSIGN(SnapshotFactory);

        Snapshot toSnapshot(const Buffer& buffer);

        void addStrip(size_t offset, ByteArray::Borrowed& content);

        size_t _stride;
        size_t _size;

        Vector<Strip> _strips;
    };

public:
//  [[script::bindings::auto]]
    Buffer(Buffer::UsageBits usageBits, sp<Uploader> uploader);
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

//  [[script::bindings::auto]]
    sp<ByteArray> synchronize(size_t offset, size_t size, sp<Boolean> canceled);

    void upload(GraphicsContext&) const;

    const sp<Delegate>& delegate() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Buffer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Buffer> build(const Scope& args) override;

    private:
        SafeBuilder<Uploader> _uploader;
        Usage _usage;
    };

private:
    sp<Delegate> _delegate;
};

}
