#ifndef ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_
#define ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetDrawCompute : public Snippet {
public:
    SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer, sp<Integer> numWorkGroups);

    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;

//  [[plugin::builder("compute")]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _buffer;
        sp<Builder<Integer>> _num_work_groups;
    };

private:
    sp<Shader> _shader;
    sp<Buffer> _buffer;
    sp<Integer> _num_work_groups;

    sp<ShaderBindings> _shader_bindings;
};

}

#endif
