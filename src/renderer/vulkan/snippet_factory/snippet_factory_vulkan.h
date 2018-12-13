#ifndef ARK_RENDERER_VULKAN_SNIPPET_FACTORY_SNIPPET_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_SNIPPET_FACTORY_SNIPPET_FACTORY_VULKAN_H_

#include "renderer/inf/snippet_factory.h"

namespace ark {
namespace vulkan {

class SnippetFactoryVulkan : public SnippetFactory {
public:
    virtual sp<Snippet> createCoreSnippet(ResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings) override;
};

}
}

#endif
