#ifndef ARK_RENDERER_BASE_DRAWING_BUFFER_H_
#define ARK_RENDERER_BASE_DRAWING_BUFFER_H_

#include <vector>
#include <map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_object.h"
#include "graphics/base/transform.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API DrawingBuffer {
public:
    DrawingBuffer(const sp<ShaderBindings>& shaderBindings, size_t renderObjectCount, uint32_t stride);
    DEFAULT_COPY_AND_ASSIGN(DrawingBuffer);

    void writePosition(const V3& position);
    void writePosition(float x, float y, float z);
    void writeNormal(float x, float y, float z);
    void writeNormal(const V3& normal);
    void writeTangent(float x, float y, float z);
    void writeTangent(const V3& tangent);
    void writeBitangent(float x, float y, float z);
    void writeBitangent(const V3& bitangent);

    void writeTexCoordinate(uint16_t u, uint16_t v);

    void writeModelId(int32_t modelId);

    void nextVertex();
    void nextModel();

    void setTranslate(const V3& translate);
    void setRenderObject(const RenderObject::Snapshot& renderObject);

    const sp<ShaderBindings>& shaderBindings() const;

    const Buffer::Builder& vertices() const;
    Buffer::Builder& vertices();

    const Buffer::Snapshot& indices() const;
    void setIndices(Buffer::Snapshot indices);

    bool isInstanced() const;

    Buffer::Builder& getInstancedArrayBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, Buffer::Snapshot>> makeDividedBufferSnapshots() const;

private:
    void applyVaryings();

private:
    sp<ShaderBindings> _shader_bindings;
    sp<PipelineBindings> _pipeline_bindings;

    Buffer::Builder _vertices;
    std::map<uint32_t, Buffer::Builder> _divided_buffer_builders;

    Buffer::Snapshot _indices;

    element_index_t _indice_base;

    V3 _translate;

    Varyings::Snapshot _varyings;

    bool _is_instanced;
    const Transform::Snapshot* _transform;
};

}

#endif
