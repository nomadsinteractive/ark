#include "bgfx/impl/renderer_factory/renderer_factory_bgfx.h"

#include <bgfx/bgfx.h>

#include "core/util/uploader_type.h"
#include "core/util/log.h"

#include "graphics/base/size.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/inf/snippet_factory.h"
#include "renderer/inf/snippet.h"

#include "app/base/application_context.h"

#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"
#include "bgfx/impl/render_view/render_view_bgfx.h"
#include "bgfx/impl/texture/texture_bgfx.h"
#include "bgfx/base/resource_base.h"

namespace ark::plugin::bgfx {

namespace {

::bgfx::Attrib::Enum toAttribEnum(Attribute::LayoutType layoutType, uint32_t customAttrIdx)
{
    switch(layoutType)
    {
        case Attribute::LAYOUT_TYPE_POSITION:
            return ::bgfx::Attrib::Position;
        case Attribute::LAYOUT_TYPE_TEX_COORD:
            return ::bgfx::Attrib::TexCoord0;
        case Attribute::LAYOUT_TYPE_COLOR:
            return ::bgfx::Attrib::Color0;
        case Attribute::LAYOUT_TYPE_NORMAL:
            return ::bgfx::Attrib::Normal;
        case Attribute::LAYOUT_TYPE_TANGENT:
            return ::bgfx::Attrib::Tangent;
        case Attribute::LAYOUT_TYPE_BITANGENT:
            return ::bgfx::Attrib::Bitangent;
        default:
            break;
    }
    CHECK(customAttrIdx < 10, "Too many custom attributes");
    return static_cast<::bgfx::Attrib::Enum>(customAttrIdx < 3 ? ::bgfx::Attrib::Color1 + customAttrIdx : ::bgfx::Attrib::TexCoord1 + (customAttrIdx - 3));
}

::bgfx::AttribType::Enum toAttribType(Attribute::Type type)
{
    switch(type)
    {
        case Attribute::TYPE_BYTE:
            return ::bgfx::AttribType::Uint8;
        case Attribute::TYPE_FLOAT:
            return ::bgfx::AttribType::Float;
        case Attribute::TYPE_INTEGER:
            return ::bgfx::AttribType::Float;
        case Attribute::TYPE_SHORT:
            return ::bgfx::AttribType::Int16;
        case Attribute::TYPE_UBYTE:
            return ::bgfx::AttribType::Uint8;
        case Attribute::TYPE_USHORT:
            return ::bgfx::AttribType::Int16;
        default:
            break;
    }
    return ::bgfx::AttribType::Float;
}

void setupVertexBufferLayout(::bgfx::VertexLayout& vertexBufLayout, const PipelineDescriptor& pipelineDescriptor)
{
    vertexBufLayout.begin();
    uint32_t customAttrIdx = 0;
    for(const auto& [k, v] : pipelineDescriptor.input()->getStreamLayout(0).attributes())
    {
        const ::bgfx::Attrib::Enum attribEnum = toAttribEnum(v.layoutType(), v.layoutType() == Attribute::LAYOUT_TYPE_CUSTOM ? customAttrIdx++ : customAttrIdx);
        vertexBufLayout.add(attribEnum, v.length(), toAttribType(v.type()), v.normalized());
    }
    vertexBufLayout.end();
}

class StaticVertexBufferBgfx final : public ResourceBase<::bgfx::VertexBufferHandle, Buffer::Delegate> {
public:

    void setupLayout(const PipelineDescriptor& pipelineDescriptor) override
    {
        setupVertexBufferLayout(_vertex_buffer_layout, pipelineDescriptor);
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(_handle)
            _handle.destroy();

        DASSERT(_vertex_buffer_layout.m_stride > 0);
        DASSERT(_size <= _data.size());
        _handle.reset(::bgfx::createVertexBuffer(::bgfx::makeRef(_data.data(), _size), _vertex_buffer_layout));
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        CHECK(!::bgfx::isValid(_handle), "Cannot upload data into a static vertex buffer");
        _data = UploaderType::toBytes(input);
        _size = _data.size();
        upload(graphicsContext);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _data;
    ::bgfx::VertexLayout _vertex_buffer_layout;
};

class DynamicVertexBufferBgfx final : public ResourceBase<::bgfx::DynamicVertexBufferHandle, Buffer::Delegate> {
public:

    void setupLayout(const PipelineDescriptor& pipelineDescriptor) override
    {
        setupVertexBufferLayout(_vertex_buffer_layout, pipelineDescriptor);
    }

    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
            _handle.reset(::bgfx::createDynamicVertexBuffer(_size, _vertex_buffer_layout));

        DASSERT(_size <= _data.size());
        ::bgfx::update(_handle, 0, ::bgfx::makeRef(_data.data(), _size));
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        _data = UploaderType::toBytes(input);
        _size = _data.size();
        upload(graphicsContext);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _data;
    ::bgfx::VertexLayout _vertex_buffer_layout;
};

class StaticIndexBuffer final : public ResourceBase<::bgfx::IndexBufferHandle, Buffer::Delegate> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(_handle)
            _handle.destroy();

        DASSERT(_size <= _indices.size());
        _handle.reset(::bgfx::createIndexBuffer(::bgfx::makeRef(_indices.data(), _size)));
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        CHECK(!::bgfx::isValid(_handle), "Cannot upload data into a static index buffer");
        _indices = UploaderType::toBytes(input);
        _size = _indices.size();
        upload(graphicsContext);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _indices;
};

class DynamicIndexBuffer final : public ResourceBase<::bgfx::DynamicIndexBufferHandle, Buffer::Delegate> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
            _handle.reset(::bgfx::createDynamicIndexBuffer(_size));

        DASSERT(_size <= _indices.size());
        ::bgfx::update(_handle, 0, ::bgfx::makeRef(_indices.data(), _size));
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        _indices = UploaderType::toBytes(input);
        _size = _indices.size();
        upload(graphicsContext);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _indices;
};

class IndirectBuffer final : public ResourceBase<::bgfx::IndirectBufferHandle, Buffer::Delegate> {
public:
    void upload(GraphicsContext& graphicsContext) override
    {
        if(!_handle)
            _handle.reset(::bgfx::createIndirectBuffer(_size));

        DASSERT(_size <= _indices.size());
    }

    void uploadBuffer(GraphicsContext& graphicsContext, Uploader& input) override
    {
        _indices = UploaderType::toBytes(input);
        _size = _indices.size();
        upload(graphicsContext);
    }

    void downloadBuffer(GraphicsContext& graphicsContext, size_t offset, size_t size, void* ptr) override
    {
        FATAL("Unimplemented");
    }

private:
    std::vector<uint8_t> _indices;
};

class SnippetBgfx final : public Snippet {
public:

    void preInitialize(PipelineBuildingContext& context) override
    {
    }
    void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override
    {
    }

    sp<DrawEvents> makeDrawEvents(const RenderRequest&) override
    {
        return sp<DrawEvents>::make();
    }

    sp<DrawEvents> makeDrawEvents() override
    {
        return sp<DrawEvents>::make();
    }
};

class SnippetFactoryBgfx final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override
    {
        return sp<Snippet>::make<SnippetBgfx>();
    }
};

void setVersion(Ark::RendererVersion version, RenderEngineContext& vkContext)
{
    LOGD("Choose Bgfx Version = %d", version);
    std::map<String, String>& definitions = vkContext.definitions();

    definitions["vert.in"] = "in";
    definitions["vert.out"] = "out";
    definitions["frag.in"] = "in";
    definitions["frag.out"] = "out";
    definitions["frag.color"] = "f_FragColor";
    vkContext.setSnippetFactory(sp<SnippetFactoryBgfx>::make());

    vkContext.setVersion(version);
}

}

RendererFactoryBgfx::RendererFactoryBgfx()
    : RendererFactory(Ark::COORDINATE_SYSTEM_RHS)
{
}

void RendererFactoryBgfx::onSurfaceCreated(RenderEngine& renderEngine)
{
    const RenderEngine::PlatformInfo& info = Ark::instance().applicationContext()->renderEngine()->info();
    Ark::RendererVersion version = renderEngine.version();
    setVersion(version == Ark::RENDERER_VERSION_AUTO ? Ark::RENDERER_VERSION_VULKAN_13 : version, renderEngine.context());

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

sp<RenderEngineContext> RendererFactoryBgfx::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    return renderContext;
}

sp<Buffer::Delegate> RendererFactoryBgfx::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    switch(type)
    {
        case Buffer::TYPE_VERTEX:
            return usage == Buffer::USAGE_STATIC ? sp<Buffer::Delegate>::make<StaticVertexBufferBgfx>() : sp<Buffer::Delegate>::make<DynamicVertexBufferBgfx>();
        case Buffer::TYPE_INDEX:
            return usage == Buffer::USAGE_STATIC ? sp<Buffer::Delegate>::make<StaticIndexBuffer>() : sp<Buffer::Delegate>::make<DynamicIndexBuffer>();
        case Buffer::TYPE_DRAW_INDIRECT:
            return sp<Buffer::Delegate>::make<DynamicVertexBufferBgfx>();
        case Buffer::TYPE_STORAGE:
            return sp<Buffer::Delegate>::make<DynamicVertexBufferBgfx>();
        default:
            FATAL("Unknow buffer type: %d", type);
            break;
    }
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
    return sp<PipelineFactory>::make<PipelineFactoryBgfx>();
}

sp<RenderView> RendererFactoryBgfx::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::make<RenderViewBgfx>(renderContext, renderController);
}

sp<Texture::Delegate> RendererFactoryBgfx::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D || parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<Texture::Delegate>::make<TextureBgfx>(parameters->_type, static_cast<uint32_t>(size->widthAsFloat()), static_cast<uint32_t>(size->heightAsFloat()), std::move(parameters));
    return nullptr;
}

sp<RendererFactory> RendererFactoryBgfx::BUILDER::build(const Scope& args)
{
    return sp<RendererFactory>::make<RendererFactoryBgfx>();
}

}
