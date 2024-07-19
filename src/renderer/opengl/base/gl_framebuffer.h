#pragma once

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark::opengl {

class GLFramebuffer : public Resource {
public:
    GLFramebuffer(sp<Recycler> recycler, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments);
    ~GLFramebuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext) override;
    virtual ResourceRecycleFunc recycle() override;

private:
    sp<Recycler> _recycler;
    std::vector<sp<Texture>> _color_attachments;
    sp<Texture> _depth_stencil_attachment;

    uint32_t _id;
};

}
