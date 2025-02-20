#include "opengl/impl/renderer_factory/renderer_factory_opengl.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/camera.h"
#include "graphics/components/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/render_target.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture.h"

#include "opengl/base/gl_buffer.h"
#include "opengl/base/gl_cubemap.h"
#include "opengl/base/gl_framebuffer.h"
#include "opengl/base/gl_texture_2d.h"
#include "opengl/impl/pipeline_factory/pipeline_factory_opengl.h"
#include "opengl/impl/renderer/gl_framebuffer_renderer.h"
#include "opengl/impl/render_view/render_view_opengl.h"
#include "opengl/impl/es30/snippet_factory/snippet_factory_gles30.h"

#include "generated/opengl_plugin.h"

#include "platform/platform.h"
#include "platform/gl/gl.h"

namespace ark::plugin::opengl {

namespace {

void setVersion(Enum::RendererVersion version, RenderEngineContext& glContext)
{
    LOGD("Choose GLVersion = %d", version);
    glContext.setSnippetFactory(sp<SnippetFactory>::make<SnippetFactoryGLES30>());
    glContext.setVersion(version);
}

int32_t toClearMaskBits(const RenderTarget::CreateConfigure& configure)
{
    RenderTarget::ClearBitSet clearMask = configure._clear_bits;
    if(configure._depth_stencil_usage.has(RenderTarget::DEPTH_STENCIL_USAGE_FOR_INPUT))
        clearMask.set(RenderTarget::CLEAR_BIT_DEPTH_STENCIL, false);
    return clearMask.bits();
}

}

RendererFactoryOpenGL::RendererFactoryOpenGL()
    : RendererFactory({{Enum::RENDERING_BACKEND_BIT_OPENGL}, Ark::COORDINATE_SYSTEM_RHS, true, sizeof(float)})
{
}

sp<RenderEngineContext> RendererFactoryOpenGL::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    if(renderer._version != Enum::RENDERER_VERSION_AUTO)
        setVersion(renderer._version, renderContext);
    return renderContext;
}

void RendererFactoryOpenGL::onSurfaceCreated(RenderEngine& renderEngine)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    glbinding::Binding::initialize(nullptr);

    if(renderEngine.version() == Enum::RENDERER_VERSION_AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        setVersion(static_cast<Enum::RendererVersion>(glMajorVersion * 10 + glMinorVersion), renderEngine.context());
    }
}

sp<RenderView> RendererFactoryOpenGL::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewOpenGL>(renderContext, renderController);
}

sp<Buffer::Delegate> RendererFactoryOpenGL::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    return sp<Buffer::Delegate>::make<GLBuffer>(type, usage, Ark::instance().renderController()->recycler());
}

sp<Camera::Delegate> RendererFactoryOpenGL::createCamera(Ark::RendererCoordinateSystem rcs)
{
    return rcs == Ark::COORDINATE_SYSTEM_LHS ?  sp<Camera::Delegate>::make<Camera::DelegateLH_NO>() : sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<RenderTarget> RendererFactoryOpenGL::createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure)
{
    CHECK(!configure._color_attachments.empty(), "Framebuffer object should have at least one color attachment");
    int32_t width = configure._color_attachments.at(0)->width();
    int32_t height = configure._color_attachments.at(0)->height();
    uint32_t drawBufferCount = static_cast<uint32_t>(configure._color_attachments.size());
    sp<GLFramebuffer> fbo = sp<GLFramebuffer>::make(Ark::instance().renderController()->recycler(), std::move(configure));
    return sp<RenderTarget>::make(sp<GLFramebufferRenderer>::make(fbo, width, height, std::move(renderer), drawBufferCount, toClearMaskBits(configure)), std::move(fbo));
}

sp<PipelineFactory> RendererFactoryOpenGL::createPipelineFactory()
{
    return sp<PipelineFactory>::make<PipelineFactoryOpenGL>();
}

sp<Texture::Delegate> RendererFactoryOpenGL::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D)
        return sp<GLTexture2D>::make(Ark::instance().renderController()->recycler(), std::move(size), std::move(parameters));
    if(parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<GLCubemap>::make(Ark::instance().renderController()->recycler(), std::move(size), std::move(parameters));
    DFATAL("Unsupported texture type: %d", parameters->_type);
    return nullptr;
}

sp<RendererFactory> RendererFactoryOpenGL::BUILDER::build(const Scope& /*args*/)
{
    return sp<RendererFactory>::make<RendererFactoryOpenGL>();
}

}
