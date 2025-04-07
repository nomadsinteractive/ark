#pragma once

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/resource.h"

#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

class GLBuffer final : public Buffer::Delegate {
public:
    GLBuffer(Buffer::Usage usage, sp<Recycler> recycler);
    ~GLBuffer() override;

    uint64_t id() override;
    void upload(GraphicsContext& graphicsContext) override;
    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& uploader) override;
    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override;
    ResourceRecycleFunc recycle() override;

private:
    void doUpload(GraphicsContext& graphicsContext, Uploader& uploader);

private:
    GLenum _type;
    GLenum _usage;

    sp<Recycler> _recycler;

    uint32_t _id;
};

}
