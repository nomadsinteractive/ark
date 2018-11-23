#ifndef ARK_RENDERER_OPENGL_ES20_RENDERER_SHADER_FRAME_H_
#define ARK_RENDERER_OPENGL_ES20_RENDERER_SHADER_FRAME_H_

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
#include "renderer/base/gl_buffer.h"

namespace ark {

//[[core::class]]
class ShaderFrame : public Renderer, public Block {
public:
    ShaderFrame(const sp<Size>& size, const sp<GLPipeline>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const SafePtr<Size>& size() override;

//  [[plugin::resource-loader("shader-frame")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        document _manifest;

        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Size>> _size;
        sp<Builder<GLPipeline>> _shader;
    };

private:
    bytearray getArrayBuffer(float x, float y) const;

private:
    SafePtr<Size> _size;
    sp<GLResourceManager> _resource_manager;
    sp<GLPipeline> _shader;

    sp<ObjectPool> _object_pool;
    sp<MemoryPool> _memory_pool;
    sp<GLShaderBindings> _shader_bindings;
    GLBuffer _array_buffer;
};

}

#endif
