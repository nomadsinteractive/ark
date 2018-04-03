#include "renderer/impl/gl_model/gl_model_line.h"

#include "renderer/base/gl_attribute.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_shader.h"

namespace ark {

GLModelLine::GLModelLine(const sp<GLShader>& shader, const sp<Atlas>& atlas)
    : _atlas(atlas), _tex_coordinate_offset(shader->getAttribute("TexCoordinate").offset()), _stride(shader->stride())
{
    DCHECK(_tex_coordinate_offset % 2 == 0 && _tex_coordinate_offset < _stride, "Illegal TexCoordinate offset: %d, stride: %d", _tex_coordinate_offset, _stride);
    _tex_coordinate_offset /= 2;
}

bytearray GLModelLine::getArrayBuffer(MemoryPool& memoryPool, const LayerContext::Snapshot& renderContext, float x, float y)
{
    return nullptr;
}

GLBuffer GLModelLine::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext)
{
    return GLBuffer();
}

uint32_t GLModelLine::mode() const
{
    return static_cast<uint32_t>(GL_TRIANGLE_STRIP);
}

}
