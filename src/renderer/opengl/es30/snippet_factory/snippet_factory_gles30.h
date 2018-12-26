#ifndef ARK_RENDERER_OPENGL_ES30_SNIPPET_FACTORY_SNIPPET_FACTORY_GLES30_H_
#define ARK_RENDERER_OPENGL_ES30_SNIPPET_FACTORY_SNIPPET_FACTORY_GLES30_H_

#include "renderer/inf/snippet_factory.h"

namespace ark {
namespace gles30 {

class SnippetFactoryGLES30 : public SnippetFactory {
public:
    virtual sp<Snippet> createCoreSnippet(ResourceManager& resourceManager, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings) override;
};

}
}

#endif
