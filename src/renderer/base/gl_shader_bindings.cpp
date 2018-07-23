#include "renderer/base/gl_shader_bindings.h"

#include "renderer/base/gl_program.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_snippet_delegate.h"

namespace ark {

GLShaderBindings::GLShaderBindings(const sp<GLShader>& shader, const GLBuffer& arrayBuffer)
    : _shader(shader), _snippet(sp<GLSnippetDelegate>::make(shader)), _array_buffer(arrayBuffer)
{
}

const sp<GLShader>& GLShaderBindings::shader() const
{
    return _shader;
}

const sp<GLSnippetDelegate>& GLShaderBindings::snippet() const
{
    return _snippet;
}

const GLBuffer& GLShaderBindings::arrayBuffer() const
{
    return _array_buffer;
}

void GLShaderBindings::setInstancedArrayBuffer(uint32_t divisor, const GLBuffer& buffer)
{
    DCHECK(divisor, "Cannot bind non-instanced array buffer to instanced array buffers");
    _instanced_array_buffers[divisor] = buffer;
}

void GLShaderBindings::bindArrayBuffers(GraphicsContext& graphicsContext, GLProgram& program) const
{
    DCHECK(program.id(), "GLProgram unprepared");
    bindAttributesByDivisor(graphicsContext, program, 0);
    for(const auto iter : _instanced_array_buffers)
    {
        glBindBuffer(GL_ARRAY_BUFFER, iter.second.id());
        bindAttributesByDivisor(graphicsContext, program, iter.first);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void GLShaderBindings::bindAttributesByDivisor(GraphicsContext& /*graphicsContext*/, GLProgram& program, uint32_t divisor) const
{
    for(const auto& i : _shader->_source->_attributes)
    {
        const GLAttribute& attr = i.second;
        if(attr.divisor() == divisor)
        {
            const GLProgram::Attribute& glAttribute = program.getAttribute(attr.name());
            attr.setVertexPointer(glAttribute.location(), _shader->_source->_stride[divisor]);
        }
    }
}

}
