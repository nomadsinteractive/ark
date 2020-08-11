#ifndef ARK_RENDERER_OPENGL_BASE_GL_FRAMEBUFFER_H_
#define ARK_RENDERER_OPENGL_BASE_GL_FRAMEBUFFER_H_

#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {
namespace opengl {

class GLFramebuffer : public Resource {
public:
    GLFramebuffer(const sp<Recycler>& recycler, std::vector<sp<Texture>> colorAttachments, std::vector<sp<Texture>> renderBufferAttachments);
    ~GLFramebuffer() override;

    virtual uint64_t id() override;

    virtual void upload(GraphicsContext& graphicsContext, const sp<Uploader>& uploader) override;
    virtual RecycleFunc recycle() override;

private:
    sp<Recycler> _recycler;
    std::vector<sp<Texture>> _color_attachments;
    std::vector<sp<Texture>> _render_buffer_attachments;

    uint32_t _id;
};

}
}

#endif
