#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"

#include "renderer/base/texture.h"
#include "renderer/base/render_context.h"
#include "renderer/base/render_controller.h"

#include "renderer/opengl/base/gl_buffer.h"
#include "renderer/opengl/base/gl_texture_2d.h"
#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"
#include "renderer/opengl/render_view/render_view_opengl.h"
#include "renderer/opengl/es20/snippet_factory/snippet_factory_gles20.h"
#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"
#include "renderer/opengl/util/gl_util.h"

#include "generated/opengl_plugin.h"

#include "platform/platform.h"
#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

RendererFactoryOpenGL::RendererFactoryOpenGL(const sp<Recycler>& recycler)
    : _recycler(recycler)
{
    const Global<PluginManager> pluginManager;
    pluginManager->addPlugin(sp<opengl::OpenglPlugin>::make());
}

sp<RenderContext> RendererFactoryOpenGL::initialize(Ark::RendererVersion version)
{
    const sp<RenderContext> renderContext = sp<RenderContext>::make(version, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    if(version != Ark::AUTO)
        setVersion(version, renderContext);
    return renderContext;
}

void RendererFactoryOpenGL::onSurfaceCreated(RenderContext& glContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    Platform::glInitialize();

    if(glContext.version() == Ark::AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        if(glMajorVersion != 0)
            setVersion(static_cast<Ark::RendererVersion>(glMajorVersion * 10 + glMinorVersion), glContext);
        else
            setVersion(Ark::OPENGL_20, glContext);
    }
}

void RendererFactoryOpenGL::setVersion(Ark::RendererVersion version, RenderContext& glContext)
{
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& annotations = glContext.annotations();
    if(version == Ark::OPENGL_20 || version == Ark::OPENGL_21)
    {
        annotations["vert.in"] = "attribute";
        annotations["vert.out"] = "varying";
        annotations["frag.in"] = "varying";
        annotations["frag.out"] = "varying";
        annotations["frag.color"] = "gl_FragColor";
        glContext.setSnippetFactory(sp<gles20::SnippetFactoryGLES20>::make());
    }
    else
    {
        annotations["vert.in"] = "in";
        annotations["vert.out"] = "out";
        annotations["frag.in"] = "in";
        annotations["frag.out"] = "out";
        annotations["frag.color"] = "v_FragColor";
        glContext.setSnippetFactory(sp<gles30::SnippetFactoryGLES30>::make());
    }
    glContext.setVersion(version);
}

sp<RenderView> RendererFactoryOpenGL::createRenderView(const sp<RenderContext>& renderContext, const sp<RenderController>& renderController, const Viewport& viewport)
{
    return sp<RenderView>::adopt(new RenderViewOpenGL(renderContext, renderController, viewport));
}

sp<Buffer::Delegate> RendererFactoryOpenGL::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    return sp<GLBuffer>::make(type, usage, _recycler);
}

sp<PipelineFactory> RendererFactoryOpenGL::createPipelineFactory()
{
    return sp<PipelineFactoryOpenGL>::make();
}

sp<Texture> RendererFactoryOpenGL::createTexture(uint32_t width, uint32_t height, const sp<Variable<bitmap>>& bitmap)
{
    const sp<Size> size = sp<Size>::make(static_cast<float>(width), static_cast<float>(height));
    const sp<GLTexture2D> texture2d = sp<GLTexture2D>::make(_recycler, size, sp<Texture::Parameters>::make(), bitmap);
    return sp<Texture>::make(size, sp<Variable<sp<Resource>>::Const>::make(texture2d), Texture::TYPE_2D);
}

}
}
