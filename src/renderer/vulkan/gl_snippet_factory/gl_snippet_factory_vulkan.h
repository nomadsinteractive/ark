#ifndef ARK_RENDERER_VULKAN_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_VULKAN_H_
#define ARK_RENDERER_VULKAN_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_VULKAN_H_

#include "renderer/inf/gl_snippet_factory.h"

namespace ark {
namespace vulkan {

class GLSnippetFactoryVulkan : public GLSnippetFactory {
public:
    virtual sp<GLSnippet> createCoreGLSnippet(GLResourceManager& glResourceManager, const GLShader& shader, const sp<GLShaderBindings>& shaderBindings) override;
};

}
}

#endif
