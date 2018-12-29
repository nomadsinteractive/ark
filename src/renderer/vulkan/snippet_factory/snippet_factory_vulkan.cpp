#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"

#include "renderer/base/resource_manager.h"

#include "renderer/inf/snippet.h"

namespace ark {
namespace vulkan {

namespace {

class CoreSnippetVulkan : public Snippet {

};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet(ResourceManager& resourceManager, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings)
{
    return sp<CoreSnippetVulkan>::make();
}

}
}
