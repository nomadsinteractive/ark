#ifndef ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_
#define ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_

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
#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/varyings.h"

namespace ark {

class ARK_API GLModelBuffer {
public:
    GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<GLShaderBindings>& shaderBindings, size_t renderObjectCount, uint32_t stride);
    DEFAULT_COPY_AND_ASSIGN(GLModelBuffer);

    void setPosition(float x, float y, float z);
    void setTexCoordinate(uint16_t u, uint16_t v);
    void setNormal(const V3& normal);
    void setTangents(const V3& tangents);

    void nextVertex();
    void nextModel();

    void setTranslate(const V3& translate);
    void setRenderObject(const RenderObject::Snapshot& renderObject);

    const Transform::Snapshot& transform() const;

    const GLBuffer::Builder& vertices() const;
    GLBuffer::Builder& vertices();

    const GLBuffer::Snapshot& indices() const;
    void setIndices(GLBuffer::Snapshot indices);

    bool isInstanced() const;

    GLBuffer::Builder& getInstancedArrayBuilder(uint32_t divisor);

    std::vector<std::pair<uint32_t, GLBuffer::Snapshot>> makeInstancedBufferSnapshots() const;

private:
    void applyVaryings();

private:
    sp<GLShaderBindings> _shader_bindings;

    GLBuffer::Builder _vertices;
    std::map<uint32_t, GLBuffer::Builder> _instanced_buffer_builders;

    GLBuffer::Snapshot _indices;

    glindex_t _indice_base;

    Transform::Snapshot _transform;
    V3 _translate;

    Varyings::Snapshot _varyings;

    bool _is_instanced;
};

}

#endif
