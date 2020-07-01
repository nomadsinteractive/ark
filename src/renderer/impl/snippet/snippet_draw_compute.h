#ifndef ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_
#define ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetDrawCompute : public SnippetDraw {
public:
    SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer);

    virtual void postDraw(GraphicsContext& graphicsContext) override;

//  [[plugin::builder("compute")]]
    class BUILDER : public Builder<SnippetDraw> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<SnippetDraw> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _buffer;

    };

private:
    sp<Shader> _shader;
    sp<Buffer> _buffer;

    sp<ShaderBindings> _shader_bindings;

};

}

#endif
