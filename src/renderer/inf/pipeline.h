#ifndef ARK_RENDERER_INF_PIPELINE_H_
#define ARK_RENDERER_INF_PIPELINE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/texture.h"
#include "renderer/inf/resource.h"
#include "renderer/inf/render_model.h"

namespace ark {

class ARK_API Pipeline : public Resource {
public:
    virtual ~Pipeline() = default;

    virtual void active(GraphicsContext& graphicsContext, const PipelineInput& input) = 0;

    virtual void bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings) = 0;
    virtual void activeTexture(Resource& texture, Texture::Type type, uint32_t name) = 0;
};

}

#endif
