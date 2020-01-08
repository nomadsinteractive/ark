#ifndef ARK_RENDERER_BASE_BUFFER_H_
#define ARK_RENDERER_BASE_BUFFER_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
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
        TYPE_COUNT
    };

    enum Usage {
        USAGE_DYNAMIC,
        USAGE_STATIC,
        USAGE_COUNT
    };

    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_BITANGENT,
        ATTRIBUTE_NAME_MODEL_ID,
        ATTRIBUTE_NAME_COUNT
    };

    struct Attributes {
        Attributes(const PipelineInput& input);

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
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

    struct Block {
        Block(size_t offset, const ByteArray::Borrowed& content);

        size_t offset;
        ByteArray::Borrowed content;
    };

    class ARK_API Builder {
    public:
        Builder(const RenderRequest& renderRequest, const Attributes& attributes, size_t stride);
        DEFAULT_COPY_AND_ASSIGN(Builder);

        void setGrowCapacity(size_t growCapacity);

        Snapshot toSnapshot(const Buffer& buffer) const;

        void addBlock(size_t offset, ByteArray::Borrowed& content);

        size_t length() const;

        sp<Uploader> makeUploader() const;

        RenderRequest _render_request;
        Attributes _attributes;

        size_t _stride;
        size_t _grow_capacity;

        std::vector<Block> _blocks;

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
