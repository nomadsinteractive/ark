#include "bgfx/impl/renderer_factory/renderer_factory_bgfx.h"

#include <bgfx/bgfx.h>

#include "core/util/uploader_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "renderer/base/render_engine_context.h"

namespace ark::plugin::bgfx {

namespace {

template<typename T> class BGFXResourceBase : public Buffer::Delegate {
public:
    uint64_t id() override
    {
        return _buffer.idx;
    }

    ResourceRecycleFunc recycle() override
    {
        T vb = _buffer;
        return [vb](GraphicsContext& context) {
            ::bgfx::destroy(vb);
        };
    }

protected:
    T _buffer = { ::bgfx::kInvalidHandle };
};

class BufferDelegateVertex final : public virtual BGFXResourceBase<::bgfx::VertexBufferHandle> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_buffer))
        {
            DASSERT(_size <= _data.size());
            _buffer = ::bgfx::createVertexBuffer(::bgfx::makeRef(_data.data(), _size), _vertex_buffer_layout);
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

class BufferDelegateIndex final : public virtual BGFXResourceBase<::bgfx::IndexBufferHandle> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!::bgfx::isValid(_buffer))
        {
            DASSERT(_size <= _indices.size());
            _buffer = ::bgfx::createIndexBuffer(::bgfx::makeRef(_indices.data(), _size));
        }
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        if(!::bgfx::isValid(_buffer))
        {
            _indices = UploaderType::toBytes(input);
            _buffer = ::bgfx::createIndexBuffer(::bgfx::makeRef(_indices.data(), _size));
        }
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _indices;
};

class TextureDelegateBgfx final : public BGFXResourceBase<::bgfx::TextureHandle>, public Texture::Delegate {
public:

    void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;

    void clear(GraphicsContext& graphicsContext) override;

    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;

    void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

private:
    ::bgfx::TextureHandle _texture;
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

sp<Texture::Delegate> RendererFactoryBgfx::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    return nullptr;
}

}
