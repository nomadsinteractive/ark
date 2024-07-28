#include "bgfx/impl/renderer_factory/renderer_factory_bgfx.h"

#include <bgfx/bgfx.h>

#include "core/util/uploader_type.h"

#include "graphics/base/size.h"

#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"

#include "app/base/application_context.h"

#include "bgfx/impl/texture/texture_bgfx.h"
#include "bgfx/base/resource_base.h"


namespace ark::plugin::bgfx {

namespace {

class BufferDelegateVertex final : public ResourceBase<::bgfx::VertexBufferHandle, Buffer::Delegate> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_handle))
        {
            DASSERT(_size <= _data.size());
            _handle = ::bgfx::createVertexBuffer(::bgfx::makeRef(_data.data(), _size), _vertex_buffer_layout);
        }
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        FATAL("Cannot upload data into a static vertex");
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _data;
    ::bgfx::VertexLayout _vertex_buffer_layout;
};

class BufferDelegateIndex final : public ResourceBase<::bgfx::IndexBufferHandle, Buffer::Delegate> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_handle))
        {
            DASSERT(_size <= _indices.size());
            _handle = ::bgfx::createIndexBuffer(::bgfx::makeRef(_indices.data(), _size));
        }
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        if(!::bgfx::isValid(_handle))
        {
            _indices = UploaderType::toBytes(input);
            _handle = ::bgfx::createIndexBuffer(::bgfx::makeRef(_indices.data(), _size));
        }
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _indices;
};

}

void RendererFactoryBgfx::onSurfaceCreated(RenderEngine& renderEngine)
{
    const RenderEngine::PlatformInfo& info = Ark::instance().applicationContext()->renderEngine()->info();
    ::bgfx::Init init;
    init.type = renderEngine.version() >= Ark::RENDERER_VERSION_VULKAN ? ::bgfx::RendererType::Vulkan : ::bgfx::RendererType::OpenGL;
    init.vendorId = BGFX_PCI_ID_NONE;
#ifdef ARK_PLATFORM_WINDOWS
    init.platformData.nwh  = info.windows.window;
#elif defined(ARK_PLATFORM_DARWIN)
    init.platformData.nwh  = info.darwin.window;
#endif
    init.platformData.ndt  = nullptr;
    init.platformData.type = ::bgfx::NativeWindowHandleType::Default;
    init.resolution.width  = renderEngine.context()->displayResolution().width;
    init.resolution.height = renderEngine.context()->displayResolution().height;
    init.resolution.reset  = 0;
    ::bgfx::init(init);
}

sp<RenderEngineContext> RendererFactoryBgfx::createRenderEngineContext(Ark::RendererVersion version)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(version, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    return renderContext;
}

sp<Buffer::Delegate> RendererFactoryBgfx::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    if(type == Buffer::TYPE_VERTEX)
        return sp<Buffer::Delegate>::make<BufferDelegateVertex>();
    if(type == Buffer::TYPE_INDEX)
        return sp<Buffer::Delegate>::make<BufferDelegateIndex>();
    return nullptr;
}

sp<Camera::Delegate> RendererFactoryBgfx::createCamera()
{
    return sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<Framebuffer> RendererFactoryBgfx::createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
{
    std::vector<::bgfx::TextureHandle> textureHandles;
    ::bgfx::createFrameBuffer(textureHandles.size(), textureHandles.data(), false);
    return nullptr;
}

sp<PipelineFactory> RendererFactoryBgfx::createPipelineFactory()
{
    return nullptr;
}

sp<RenderView> RendererFactoryBgfx::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return nullptr;
}

sp<Texture::Delegate> RendererFactoryBgfx::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<Texture::Delegate>::make<TextureBgfx>(parameters->_type, static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

}
