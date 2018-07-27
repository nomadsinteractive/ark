#ifndef ARK_RENDERER_BASE_GL_SHADER_INPUT_H_
#define ARK_RENDERER_BASE_GL_SHADER_INPUT_H_

#include <map>
#include <unordered_map>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_attribute.h"

namespace ark {

class GLShaderInput {
public:
    class Stream {
    public:
        Stream();
        Stream(uint32_t divisor);
        DEFAULT_COPY_AND_ASSIGN(Stream);

        uint32_t stride() const;

        const std::unordered_map<String, GLAttribute>& attributes() const;

        bool hasAttribute(const String& name) const;
        void addAttribute(String name, GLAttribute attribute);
        const GLAttribute& getAttribute(const String& name) const;

        void align();

    private:
        uint32_t _divisor;
        uint32_t _stride;

        std::unordered_map<String, GLAttribute> _attributes;
    };

public:
    GLShaderInput() = default;

    void addAttribute(String name, GLAttribute attribute);
    void bind(GraphicsContext& graphicsContext, GLProgram& program, uint32_t divisor) const;

    std::vector<std::pair<uint32_t, GLBuffer>> makeInstancedArrays(GLResourceManager& resourceManager) const;

    const GLShaderInput::Stream& getStream(uint32_t divisor) const;

private:
    std::map<uint32_t, Stream> _streams;

    friend class GLShaderSource;
};

}

#endif
