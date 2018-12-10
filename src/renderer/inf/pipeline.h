#ifndef ARK_RENDERER_INF_PIPELINE_H_
#define ARK_RENDERER_INF_PIPELINE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Pipeline : public Resource {
public:
    virtual ~Pipeline() = default;

    virtual void use() = 0;
    virtual void bind(GraphicsContext& graphicsContext, const ShaderBindings& bindings) = 0;
    virtual void bindUniform(GraphicsContext& graphicsContext, const Uniform& uniform) = 0;
    virtual void activeTexture(Resource& texture, uint32_t target, uint32_t name) = 0;

};

}

#endif
