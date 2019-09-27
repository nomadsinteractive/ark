#ifndef ARK_RENDERER_IMPL_RENDERER_SKYBOX_H_
#define ARK_RENDERER_IMPL_RENDERER_SKYBOX_H_

#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class Skybox : public Renderer, public Block {
public:
    Skybox(const sp<Size>& size, const sp<Shader>& shader, const sp<Texture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const SafePtr<Size>& size() override;

//  [[plugin::resource-loader("skybox")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Size>> _size;
        sp<Builder<Shader>> _shader;
        sp<Builder<Texture>> _texture;
    };

private:
    SafePtr<Size> _size;
    sp<Shader> _shader;
    sp<ShaderBindings> _shader_bindings;
    sp<MemoryPool> _memory_pool;
    sp<ObjectPool> _object_pool;

    Buffer::Snapshot _index_buffer;
};

}

#endif
