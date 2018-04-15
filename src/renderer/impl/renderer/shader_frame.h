#ifndef ARK_RENDERER_GLES20_IMPL_RENDERER_SHADER_FRAME_H_
#define ARK_RENDERER_GLES20_IMPL_RENDERER_SHADER_FRAME_H_

#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

#include "renderer/base/gl_elements.h"

namespace ark {

//[[core::class]]
class ShaderFrame : public Renderer, public Block {
public:
    ShaderFrame(const sp<Size>& size, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const sp<Size>& size() override;

//  [[plugin::resource-loader("shader-frame")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        BeanFactory _parent;
        document _manifest;

        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Size>> _size;
        sp<Builder<GLShader>> _shader;
    };

private:
    sp<Size> _size;
    GLElementsRenderer _elements;
    LayerContext _render_context;
};

}

#endif
