#ifndef ARK_RENDERER_BASE_GL_SHADER_H_
#define ARK_RENDERER_BASE_GL_SHADER_H_

#include <map>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLShader {
public:
    class ARK_API Slot {
    public:
        Slot(const String& vertex, const String& fragment);
        Slot(const Slot& other) = default;

        bool operator <(const Slot& other) const;

    private:
        uint32_t _vertex_shader_hash;
        uint32_t _fragment_shader_hash;
    };

public:
    GLShader(const sp<GLShaderSource>& source);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(GLShader);

    static sp<Builder<GLShader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");
    static sp<GLShader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<GLSnippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    void use(GraphicsContext& graphicsContext);

    Slot preprocess(GraphicsContext& graphicsContext);

    void bindUniforms(GraphicsContext& graphicsContext) const;

    const sp<GLShaderInput>& input() const;
    const sp<GLShaderSource>& source() const;

    const sp<GLProgram>& program() const;
    void setProgram(const sp<GLProgram>& program);

    const sp<GLSnippet>& snippet() const;

    const sp<GLProgram>& makeGLProgram(GraphicsContext& graphicsContext);

//[[deprecated]]
    uint32_t stride() const;
//[[deprecated]]
    const GLAttribute& getAttribute(const String& name, uint32_t divisor = 0) const;

//[[script::bindings::auto]]
    sp<Varyings> makeVaryings() const;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<GLShader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLShader> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<String>> _vertex;
        sp<Builder<String>> _fragment;
        sp<Builder<GLSnippet>> _snippet;
    };

private:
    void bindAttributesByDivisor(GraphicsContext& graphicsContext, const sp<GLProgram>& program, uint32_t divisor) const;

private:
    sp<GLShaderSource> _source;
    sp<GLProgram> _program;

    friend class GLShaderBindings;
};

}

#endif
