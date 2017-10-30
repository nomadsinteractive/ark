#include "renderer/base/gl_elements.h"

#include "core/base/object_pool.h"
#include "core/types/global.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/inf/gl_model.h"

namespace ark {

namespace {

class GLSnippetWrapper : public GLSnippet {
public:
    GLSnippetWrapper(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
        : _delegate(sp<CoreGLSnippet>::make(*this, glResourceManager, shader, arrayBuffer, appendix)) {
    }

    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override {
        _delegate->preCompile(source, context);
    }

    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override {
        _delegate->preDraw(graphicsContext, shader, context);
    }

    virtual void postDraw(GraphicsContext& graphicsContext) override {
        _delegate->postDraw(graphicsContext);
    }

private:
    sp<GLSnippet> _delegate;

    friend class CoreGLSnippet;

};

class CoreGLSnippet : public GLSnippet {
public:
    CoreGLSnippet(GLSnippetWrapper& wrapper, const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
        : _wrapper(wrapper), _gl_resource_manager(glResourceManager), _shader(shader), _array_buffer(arrayBuffer), _appendix(appendix) {
    }

    virtual void preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context) override {
        const sp<GLSnippet> delegate = _wrapper._delegate;
        _wrapper._delegate = createGLSnippet();
        _wrapper.preCompile(source, context);

    }
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context) override {
        const sp<GLSnippet> delegate = _wrapper._delegate;
        _wrapper._delegate = createGLSnippet();
        _wrapper.preDraw(graphicsContext, shader, context);

    }
    virtual void postDraw(GraphicsContext& graphicsContext) override {
        const sp<GLSnippet> delegate = _wrapper._delegate;
        _wrapper._delegate = createGLSnippet();
        _wrapper.postDraw(graphicsContext);
    }
private:
    sp<GLSnippet> createGLSnippet() {
        const Global<RenderEngine> renderEngine;
        const sp<GLSnippet> coreGLSnippet = renderEngine->createCoreGLSnippet(_gl_resource_manager, _shader, _array_buffer);
        return _appendix ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(coreGLSnippet, _appendix)) : coreGLSnippet;
    }

private:
    GLSnippetWrapper& _wrapper;

    sp<GLResourceManager> _gl_resource_manager;
    sp<GLShader> _shader;
    GLBuffer _array_buffer;

    sp<GLSnippet> _appendix;
};

}

GLElements::GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader), _texture(texture), _model(model), _mode(static_cast<GLenum>(model->mode()))
    , _array_buffer(_resource_manager->createGLBuffer(nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW))
    , _render_command_pool(resourceLoaderContext->getObjectPool<RenderCommand>()), _gl_snippet(createCoreGLSnippet(shader->snippet()))
{
}

void GLElements::render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y)
{
    const GLBuffer indexBuffer = _model->getIndexBuffer(_resource_manager, renderContext);
    if(indexBuffer)
    {
        const array<uint8_t> buf = _model->getArrayBuffer(_resource_manager, renderContext, x, y);
        pipeline.add(_render_command_pool->allocate<DrawElements>(GLSnippetContext(_texture, _array_buffer.snapshot(buf), indexBuffer, _mode), _shader, _gl_snippet));
    }
}

sp<GLSnippet> GLElements::createCoreGLSnippet(const sp<GLSnippet>& glSnippet)
{
    return sp<GLSnippetWrapper>::make(_resource_manager, _shader, _array_buffer, glSnippet);
//    const Global<RenderEngine> renderEngine;
//    const sp<GLSnippet> proc = renderEngine->createCoreGLSnippet(_resource_manager, _shader, _array_buffer);
//    return glSnippet ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(proc, glSnippet)) : proc;
}

}
