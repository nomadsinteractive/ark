#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/inf/snippet.h"

namespace ark {

class ARK_API SnippetDelegate final : public Wrapper<Snippet>, public Snippet {
public:
    SnippetDelegate(sp<Snippet> snippet);

    void preInitialize(PipelineBuildingContext& context) override;
    void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override;
    sp<DrawDecorator> makeDrawDecorator() override;

};

}
