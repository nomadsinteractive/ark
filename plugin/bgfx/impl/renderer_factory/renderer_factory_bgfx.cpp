#include "bgfx/impl/renderer_factory/renderer_factory_bgfx.h"

#include <bgfx/bgfx.h>

#include "core/util/uploader_type.h"
#include "core/util/log.h"

#include "graphics/components/size.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/inf/snippet_factory.h"
#include "renderer/inf/snippet.h"
#include "renderer/impl/snippet_factory/snippet_factory_vulkan.h"
#include "renderer/util/render_util.h"

#include "app/base/application_context.h"

#include "bgfx/base/resource_base.h"
#include "bgfx/impl/buffer/dynamic_index_buffer_bgfx.h"
#include "bgfx/impl/buffer/dynamic_vertex_buffer_bgfx.h"
#include "bgfx/impl/buffer/indirect_buffer_bgfx.h"
#include "bgfx/impl/buffer/static_index_buffer_bgfx.h"
#include "bgfx/impl/buffer/static_vertex_buffer_bgfx.h"
#include "bgfx/impl/buffer/storage_buffer_bgfx.h"
#include "bgfx/impl/pipeline_factory/pipeline_factory_bgfx.h"
#include "bgfx/impl/render_view/render_view_bgfx.h"
#include "bgfx/impl/texture/texture_bgfx.h"

namespace ark::plugin::bgfx {

namespace {

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
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        const String sLocation = "location";
        const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;

        RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const ShaderLayout& shaderLayout = pipelineLayout.shaderLayout();
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
        {
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(shaderLayout.ubos().size() + shaderLayout.ssbos().size() + shaderLayout.samplers().size()));
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");
            const uint32_t bindingOffset = std::max<uint32_t>(2, shaderLayout.ubos().size() + shaderLayout.ssbos().size());
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", bindingOffset + static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()));

            uint32_t binding = 0;
            constexpr uint32_t samplerOffset = 16;
            const std::vector<String> samplerNames = fragment->_declaration_samplers.vars().keys();
            fragment->_declaration_samplers.clear();
            for(const String& k : samplerNames)
            {
                fragment->_declaration_samplers.declare("sampler", "", k + "S", Strings::sprintf("binding = %d", bindingOffset + samplerOffset + binding));
                fragment->_declaration_samplers.declare("texture2D", "", k + "T", Strings::sprintf("binding = %d", bindingOffset + binding++));
                fragment->_predefined_macros.emplace_back(Strings::sprintf("#define %s sampler2D(%sT, %sS)", k.c_str(), k.c_str(), k.c_str()));
            }
        }

        if(const ShaderPreprocessor* compute = context.computingStage().get())
        {
            const uint32_t bindingOffset = static_cast<uint32_t>(shaderLayout.ubos().size() + shaderLayout.ssbos().size());
            RenderUtil::setLayoutDescriptor(compute->_declaration_images, "binding", bindingOffset);
        }

        const ShaderPreprocessor* prestage = nullptr;
        for(auto iter = context.renderStages().begin(); iter != context.renderStages().end(); ++iter)
        {
            if(prestage)
            {
                RenderUtil::setLayoutDescriptor(prestage->_declaration_outs, iter->second->_declaration_ins, sLocation, 0);
                RenderUtil::setLayoutDescriptor(iter->second->_declaration_outs, sLocation, 0);
            }
            prestage = iter->second.get();
        }

        for(const auto& [_, v] : context.renderStages())
        {
            ShaderPreprocessor& preprocessor = v;
            preprocessor._version = 450;
            preprocessor.declareUBOStruct(shaderLayout);
            preprocessor._predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor._predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }

    sp<DrawEvents> makeDrawEvents() override {
        return sp<Snippet::DrawEvents>::make();
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
    Map<String, String>& definitions = vkContext.definitions();

    definitions["camera.uVP"] = "u_viewProj";
    definitions["camera.uView"] = "u_view";
    definitions["camera.uProjection"] = "u_proj";
    vkContext.setSnippetFactory(sp<SnippetFactory>::make<SnippetFactoryBgfx>());

    vkContext.setVersion(version);
}

}

RendererFactoryBgfx::RendererFactoryBgfx()
    : RendererFactory({{{Ark::RENDERING_BACKEND_ALL}}, Ark::COORDINATE_SYSTEM_RHS, false, 16})
{
}

void RendererFactoryBgfx::onSurfaceCreated(RenderEngine& renderEngine)
{
    ::bgfx::Init init;
    init.type = renderEngine.version() >= Ark::RENDERER_VERSION_VULKAN ? ::bgfx::RendererType::Vulkan : ::bgfx::RendererType::OpenGL;
    init.vendorId = BGFX_PCI_ID_NONE;

    const RenderEngine::PlatformInfo& info = Ark::instance().applicationContext()->renderEngine()->info();
#ifdef ARK_PLATFORM_WINDOWS
    init.platformData.nwh  = info.windows.window;
#elif defined(ARK_PLATFORM_DARWIN)
    init.platformData.nwh  = info.darwin.window;
#endif
    init.platformData.ndt  = nullptr;
    init.platformData.type = ::bgfx::NativeWindowHandleType::Default;

    const V2& resolution = Ark::instance().manifest()->rendererResolution();
    init.resolution.width  = static_cast<uint32_t>(resolution.x());
    init.resolution.height = static_cast<uint32_t>(resolution.y());
    init.resolution.reset  = 0;

    ::bgfx::init(init);

    // ::bgfx::setDebug(BGFX_DEBUG_STATS);
}

sp<RenderEngineContext> RendererFactoryBgfx::createRenderEngineContext(const ApplicationManifest::Renderer& renderer)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(renderer, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    setVersion(renderer._version == Ark::RENDERER_VERSION_AUTO ? Ark::RENDERER_VERSION_VULKAN_13 : renderer._version, renderContext);
    return renderContext;
}

sp<Buffer::Delegate> RendererFactoryBgfx::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    switch(type)
    {
        case Buffer::TYPE_VERTEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<DynamicVertexBufferBgfx>() : sp<Buffer::Delegate>::make<StaticVertexBufferBgfx>();
        case Buffer::TYPE_INDEX:
            return usage.has(Buffer::USAGE_BIT_DYNAMIC) ? sp<Buffer::Delegate>::make<DynamicIndexBufferBgfx>() : sp<Buffer::Delegate>::make<StaticIndexBufferBgfx>();
        case Buffer::TYPE_DRAW_INDIRECT:
            return sp<Buffer::Delegate>::make<IndirectBufferBgfx>();
        case Buffer::TYPE_STORAGE:
            return sp<Buffer::Delegate>::make<StorageBufferBgfx>();
        default:
            FATAL("Unknow buffer type: %d", type);
            break;
    }
    return nullptr;
}

sp<Camera::Delegate> RendererFactoryBgfx::createCamera(Ark::RendererCoordinateSystem rcs)
{
    return rcs == Ark::COORDINATE_SYSTEM_LHS ? sp<Camera::Delegate>::make<Camera::DelegateLH_NO>() :  sp<Camera::Delegate>::make<Camera::DelegateRH_NO>();
}

sp<RenderTarget> RendererFactoryBgfx::createRenderTarget(sp<Renderer> renderer, RenderTarget::CreateConfigure configure)
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
