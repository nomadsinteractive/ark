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
    enum AttributeName {
        ATTRIBUTE_NAME_TEX_COORDINATE,
        ATTRIBUTE_NAME_NORMAL,
        ATTRIBUTE_NAME_TANGENT,
        ATTRIBUTE_NAME_COUNT
    };

    struct Attributes {
        Attributes(const GLShaderInput& shader);

        int32_t _offsets[ATTRIBUTE_NAME_COUNT];
    };


public:
    GLShaderBindings(GLResourceManager& resourceManager, const sp<GLShader>& shader);

    const sp<GLShader>& shader() const;
    const sp<GLSnippetDelegate>& snippet() const;

    const GLBuffer& arrayBuffer() const;

    const Attributes& attributes() const;

    void bindArrayBuffers(GraphicsContext& graphicsContext, GLProgram& program) const;

    std::map<uint32_t, GLBuffer::Builder> makeInstancedBufferBuilders(const sp<MemoryPool>& memoryPool, const sp<ObjectPool>& objectPool, size_t instanceCount) const;

private:
    sp<GLShader> _shader;
    sp<GLSnippetDelegate> _snippet;
    Attributes _attributes;

    GLBuffer _array_buffer;
    sp<GLShaderInput> _shader_input;
    std::vector<std::pair<uint32_t, GLBuffer>> _instanced_arrays;

    friend class GLModelBuffer;
};

}

#endif
