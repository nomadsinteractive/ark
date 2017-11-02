#ifndef ARK_RENDERER_GLES20_BASE_GL_ELEMENTS_H_
#define ARK_RENDERER_GLES20_BASE_GL_ELEMENTS_H_

#include "core/base/api.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/inf/gl_snippet.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLElements {
public:
    GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext);

    void render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y);

private:
    class GLSnippetWrapper;

    class CoreGLSnippet : public GLSnippet {
    public:
        CoreGLSnippet(GLSnippetWrapper& wrapper, const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix);

        virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override;
        virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override;
        virtual void postDraw(GraphicsContext& graphicsContext) override;

    private:
        sp<GLSnippet> createGLSnippet() const;

    private:
        GLSnippetWrapper& _wrapper;

        sp<GLResourceManager> _gl_resource_manager;
        sp<GLShader> _shader;
        GLBuffer _array_buffer;

        sp<GLSnippet> _appendix;
    };

    class GLSnippetWrapper : public GLSnippet {
    public:
        GLSnippetWrapper(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix);

        virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override;
        virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override;
        virtual void postDraw(GraphicsContext& graphicsContext) override;

    private:
        sp<GLSnippet> _delegate;

        friend class CoreGLSnippet;
    };

private:
    sp<GLSnippet> createCoreGLSnippet(const sp<GLSnippet>& glSnippet) const;

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;
    sp<GLTexture> _texture;
    sp<GLModel> _model;
    GLenum _mode;

    GLBuffer _array_buffer;

    sp<ObjectPool<RenderCommand>> _render_command_pool;
    sp<GLSnippet> _gl_snippet;
};

}

#endif
