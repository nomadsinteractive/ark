#ifndef ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_
#define ARK_RENDERER_BASE_GL_MODEL_BUFFER_H_

#include <vector>

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
    GLModelBuffer(const sp<ResourceLoaderContext>& resourceLoaderContext, const size_t growCapacity, uint32_t stride, int32_t texCoordinateOffset);
    DEFAULT_COPY_AND_ASSIGN(GLModelBuffer);

    void setPosition(float x, float y, float z);
    void setTexCoordinate(uint16_t u, uint16_t v);
    void setNormal(const V3& normal);
    void setTangents(const V3& tangents);

    void nextVertex();

    void writeIndices(const glindex_t* indices, glindex_t count);

    void nextModel();

    void setTranslate(const V3& translate);
    void setRenderObject(const RenderObject::Snapshot& renderObject);

    const Transform::Snapshot& transform() const;

    const GLBuffer::Builder& vertices() const;
    GLBuffer::Builder& vertices();

    const GLBuffer::Snapshot& indices() const;
    GLBuffer::Snapshot& indices();

private:
    void applyVaryings();

private:
    GLBuffer::Builder _vertices;
    GLBuffer::Snapshot _indices;

    std::vector<glindex_t> _index_buffer;

    int32_t _tex_coordinate_offset;
    int32_t _normal_offset;
    int32_t _tangents_offset;

    glindex_t _indice_base;

    Transform::Snapshot _transform;
    V3 _translate;

    Varyings::Snapshot _varyings;

};

}

#endif
