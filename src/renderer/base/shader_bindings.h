#ifndef ARK_RENDERER_BASE_SHADER_BINDINGS_H_
#define ARK_RENDERER_BASE_SHADER_BINDINGS_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/texture.h"

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
    ShaderBindings(RenderController& renderController, const sp<Shader>& shader);
    ShaderBindings(RenderController& renderController, const sp<Shader>& shader, const Buffer& arrayBuffer);

    const sp<Shader>& shader() const;
    const sp<SnippetDelegate>& snippet() const;
    const sp<PipelineInput>& pipelineInput() const;

    const Buffer& arrayBuffer() const;
    const std::vector<std::pair<uint32_t, Buffer>>& instancedArrays() const;

    const Attributes& attributes() const;

    void bindGLTexture(const sp<Texture>& texture, uint32_t name = 0) const;

    std::map<uint32_t, Buffer::Builder> makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const;

private:
    sp<Shader> _shader;
    sp<SnippetDelegate> _snippet;
    Attributes _attributes;

    Buffer _array_buffer;
    sp<PipelineInput> _pipeline_input;
    std::vector<std::pair<uint32_t, Buffer>> _instanced_arrays;
    std::vector<sp<Texture>> _samplers;

    friend class ModelBuffer;
};

}

#endif
