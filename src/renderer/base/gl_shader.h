#ifndef ARK_RENDERER_BASE_GL_SHADER_H_
#define ARK_RENDERER_BASE_GL_SHADER_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_resource.h"

namespace ark {

class ARK_API GLShader {
public:
    GLShader(const sp<GLShaderSource>& source, const sp<Camera>& camera);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(GLShader);

    static sp<Builder<GLShader>> fromDocument(BeanFactory& factory, const document& doc, const sp<ResourceLoaderContext>& resourceLoaderContext, const String& defVertex = "shaders/default.vert", const String& defFragment = "shaders/texture.frag");
    static sp<GLShader> fromStringTable(const String& vertex = "shaders/default.vert", const String& fragment = "shaders/texture.frag", const sp<GLSnippet>& snippet = nullptr, const sp<ResourceLoaderContext>& resourceLoaderContext = nullptr);

    void use(GraphicsContext& graphicsContext);

    void bindUniforms(GraphicsContext& graphicsContext) const;

    const sp<GLShaderSource>& source() const;

    const sp<Camera>& camera() const;

    const sp<GLProgram>& program() const;
    const sp<GLProgram>& getGLProgram(GraphicsContext& graphicsContext);

    const sp<GLSnippet>& snippet() const;

    void glUpdateMVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const;
    void glUpdateVPMatrix(GraphicsContext& graphicsContext, const Matrix& matrix) const;
    void glUpdateMatrix(GraphicsContext& graphicsContext, const String& name, const Matrix& matrix) const;

//[[deprecated]]
    uint32_t stride() const;
//[[deprecated]]
    const GLAttribute& getAttribute(const String& name, uint32_t divisor = 0) const;

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
        sp<Builder<Camera>> _camera;
    };

private:
    struct Stub : public GLResource {
        Stub(const sp<GLShaderSource>& source);

        virtual uint32_t id() override;
        virtual void prepare(GraphicsContext& graphicsContext) override;
        virtual void recycle(GraphicsContext& graphicsContext) override;

        sp<GLProgram> _program;
        sp<GLShaderSource> _source;
    };

private:
    sp<Stub> _stub;
    sp<Camera> _camera;
};

}

#endif
