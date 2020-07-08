#ifndef ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_
#define ARK_RENDERER_IMPL_SNIPPET_DRAW_COMPUTE_H_

#include <array>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetDrawCompute : public Snippet {
public:
    SnippetDrawCompute(sp<Shader> shader, sp<Buffer> buffer, std::array<sp<Integer>, 3> numWorkGroups);

    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;

//  [[plugin::builder("compute")]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const Scope& args) override;

    private:
        sp<Builder<Shader>> _shader;
        sp<Builder<Buffer>> _buffer;
        std::array<sp<Builder<Integer>>, 3> _num_work_groups;
    };

private:
    sp<Shader> _shader;
    sp<Buffer> _buffer;
    std::array<sp<Integer>, 3> _num_work_groups;

    sp<ShaderBindings> _shader_bindings;
};

}

#endif
