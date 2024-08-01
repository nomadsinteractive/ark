#pragma once

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class ShaderFrame : public Renderer {
public:
    [[deprecated]]
    ShaderFrame(const sp<Size>& size, const sp<Shader>& shader, RenderController& renderController);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    const sp<Size>& size();

//  [[plugin::resource-loader("shader-frame")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        document _manifest;

        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Size>> _size;
        sp<Builder<Shader>> _shader;
    };

private:
    ByteArray::Borrowed getVertexBuffer(RenderRequest& renderRequest, const V3& position) const;

private:
    SafePtr<Size> _size;
    sp<Shader> _shader;

    sp<PipelineBindings> _pipeline_bindings;
    Buffer _vertex_buffer;
    Buffer::Snapshot _ib_snapshot;
};

}
