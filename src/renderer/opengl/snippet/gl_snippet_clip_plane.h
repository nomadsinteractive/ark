#ifndef ARK_RENDERER_IMPL_SNIPPET_CLIP_PLANE_H_
#define ARK_RENDERER_IMPL_SNIPPET_CLIP_PLANE_H_

#include "core/inf/builder.h"
#include "core/collection/list.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/inf/snippet.h"

namespace ark {

class GLSnippetClipPlane : public Snippet {
public:
    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

//  [[plugin::builder("clip-plane")]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const sp<Scope>& args) override;

    private:
        List<std::pair<uint32_t, sp<Builder<Vec4>>>> _planes;
    };

private:
    bool _enabled;

    List<std::pair<uint32_t, sp<Vec4>>> _planes;

    friend class BUILDER;
};

}

#endif
