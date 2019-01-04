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
#include "renderer/inf/render_model.h"

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
    ShaderBindings(RenderModel::Mode mode, const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout);
    ShaderBindings(RenderModel::Mode mode, const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout, const Buffer& vertexBuffer, const Buffer& indexBuffer);

    const sp<Snippet>& snippet() const;
    const sp<PipelineLayout>& pipelineLayout() const;
    const sp<PipelineInput>& pipelineInput() const;
    const std::vector<sp<Texture>>& samplers() const;

    bool isDrawInstanced() const;

    RenderModel::Mode renderMode() const;

    const Buffer& vertexBuffer() const;
    const Buffer& indexBuffer() const;

    const std::vector<std::pair<uint32_t, Buffer>>& instancedArrays() const;

    const Attributes& attributes() const;

    void bindSampler(const sp<Texture>& texture, uint32_t name = 0);

    std::map<uint32_t, Buffer::Builder> makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const;

private:
    RenderModel::Mode _render_mode;

    Attributes _attributes;

    Buffer _vertex_buffer;
    Buffer _index_buffer;

    sp<PipelineLayout> _pipeline_layout;
    sp<PipelineInput> _pipeline_input;
    std::vector<std::pair<uint32_t, Buffer>> _instanced_arrays;
    std::vector<sp<Texture>> _samplers;

    friend class ModelBuffer;
};

}

#endif
