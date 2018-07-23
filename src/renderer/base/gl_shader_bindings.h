#ifndef ARK_RENDERER_BASE_GL_SHADER_BINDINGS_H_
#define ARK_RENDERER_BASE_GL_SHADER_BINDINGS_H_

#include <map>

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class ARK_API GLShaderBindings {
public:
    GLShaderBindings(const sp<GLShader>& shader, const GLBuffer& arrayBuffer);

    const sp<GLShader>& shader() const;
    const sp<GLSnippetDelegate>& snippet() const;

    const GLBuffer& arrayBuffer() const;

    void setInstancedArrayBuffer(uint32_t divisor, const GLBuffer& buffer);

    void bindArrayBuffers(GraphicsContext& graphicsContext, GLProgram& program) const;

private:
    void bindAttributesByDivisor(GraphicsContext& /*graphicsContext*/, GLProgram& program, uint32_t divisor) const;

private:
    sp<GLShader> _shader;
    sp<GLSnippetDelegate> _snippet;

    GLBuffer _array_buffer;
    std::map<uint32_t, GLBuffer> _instanced_array_buffers;
};

}

#endif
