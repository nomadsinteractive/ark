#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/camera.h"
#include "graphics/base/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/render_target.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_buffer.h"
#include "renderer/opengl/base/gl_cubemap.h"
#include "renderer/opengl/base/gl_framebuffer.h"
#include "renderer/opengl/base/gl_texture_2d.h"
#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"
#include "renderer/opengl/renderer/gl_framebuffer_renderer.h"
#include "renderer/opengl/render_view/render_view_opengl.h"
#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"
#include "renderer/opengl/util/gl_util.h"

#include "generated/opengl_plugin.h"

#include "platform/platform.h"
#include "platform/gl/gl.h"

namespace ark::opengl {

namespace {

void setVersion(Ark::RendererVersion version, RenderEngineContext& glContext)
{
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& definitions = glContext.definitions();
    definitions["vert.in"] = "in";
    definitions["vert.out"] = "out";
    definitions["frag.in"] = "in";
    definitions["frag.out"] = "out";
    definitions["frag.color"] = "f_FragColor";
    glContext.setSnippetFactory(sp<gles30::SnippetFactoryGLES30>::make());
    glContext.setVersion(version);
}

}

RendererFactoryOpenGL::RendererFactoryOpenGL(sp<Recycler> recycler)
    : RendererFactory({Ark::COORDINATE_SYSTEM_RHS, true, sizeof(float)}), _recycler(std::move(recycler))
{
    const Global<PluginManager> pluginManager;
    pluginManager->addPlugin(sp<opengl::OpenglPlugin>::make());
}

sp<RenderEngineContext> RendererFactoryOpenGL::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    if(renderer._version != Ark::RENDERER_VERSION_AUTO)
        setVersion(renderer._version, renderContext);
    return renderContext;
}

void RendererFactoryOpenGL::onSurfaceCreated(RenderEngine& renderEngine)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    Platform::glInitialize();

    if(renderEngine.version() == Ark::RENDERER_VERSION_AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        setVersion(static_cast<Ark::RendererVersion>(glMajorVersion * 10 + glMinorVersion), renderEngine.context());
    }
}

sp<RenderView> RendererFactoryOpenGL::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::adopt(new RenderViewOpenGL(renderContext, renderController));
}

sp<Buffer::Delegate> RendererFactoryOpenGL::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    return sp<GLBuffer>::make(type, usage, _recycler);
}

sp<Camera::Delegate> RendererFactoryOpenGL::createCamera()
{
    return sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<RenderTarget> RendererFactoryOpenGL::createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure)
{
    CHECK(!configure._color_attachments.empty(), "Framebuffer object should have at least one color attachment");
    int32_t width = configure._color_attachments.at(0)->width();
    int32_t height = configure._color_attachments.at(0)->height();
    uint32_t drawBufferCount = static_cast<uint32_t>(configure._color_attachments.size());
    const int32_t clearMask = configure._clear_mask.bits();
    sp<GLFramebuffer> fbo = sp<GLFramebuffer>::make(_recycler, std::move(configure));
    return sp<RenderTarget>::make(sp<GLFramebufferRenderer>::make(fbo, width, height, std::move(renderer), drawBufferCount, clearMask), std::move(fbo));
}

sp<PipelineFactory> RendererFactoryOpenGL::createPipelineFactory()
{
    return sp<PipelineFactoryOpenGL>::make();
}

sp<Texture::Delegate> RendererFactoryOpenGL::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D)
        return sp<GLTexture2D>::make(_recycler, std::move(size), std::move(parameters));
    else if(parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<GLCubemap>::make(_recycler, std::move(size), std::move(parameters));
    DFATAL("Unsupported texture type: %d", parameters->_type);
    return nullptr;
}

}
