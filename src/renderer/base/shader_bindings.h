#ifndef ARK_RENDERER_BASE_SHADER_BINDINGS_H_
#define ARK_RENDERER_BASE_SHADER_BINDINGS_H_

#include <map>

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class ARK_API ShaderBindings {
public:
    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_BITANGENT,
        ATTRIBUTE_NAME_COUNT
    };

    struct Attributes {
        Attributes(const PipelineInput& shader);

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
    };

public:
    ShaderBindings(GLResourceManager& resourceManager, const sp<Shader>& shader);
    ShaderBindings(GLResourceManager& resourceManager, const sp<Shader>& shader, const GLBuffer& arrayBuffer);

    const sp<Shader>& shader() const;
    const sp<GLSnippetDelegate>& snippet() const;
    const sp<PipelineInput>& pipelineInput() const;

    const GLBuffer& arrayBuffer() const;
    const std::vector<std::pair<uint32_t, GLBuffer>>& instancedArrays() const;

    const Attributes& attributes() const;

    void bindGLTexture(const sp<Texture>& texture, uint32_t name = 0) const;
    void bindGLTexture(const sp<RenderResource>& texture, uint32_t target, uint32_t name) const;

    std::map<uint32_t, GLBuffer::Builder> makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const;

private:
    sp<Shader> _shader;
    sp<GLSnippetDelegate> _snippet;
    Attributes _attributes;

    GLBuffer _array_buffer;
    sp<PipelineInput> _pipeline_input;
    std::vector<std::pair<uint32_t, GLBuffer>> _instanced_arrays;

    friend class ModelBuffer;
};

}

#endif
