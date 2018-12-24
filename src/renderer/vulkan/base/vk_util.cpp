#include "renderer/vulkan/base/vk_util.h"

#include <array>

#include <glm/gtc/matrix_transform.hpp>

#include <ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>

#include "core/base/plugin_manager.h"
#include "core/inf/array.h"
#include "core/inf/dictionary.h"
#include "core/inf/variable.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/forwarding.h"
#include "graphics/base/bitmap.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "generated/vulkan_plugin.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark {

#ifdef _WIN32
    extern HINSTANCE gInstance;
    extern HWND gWnd;
#endif

namespace vulkan {

namespace {

static const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

class GLSLLangInitializer {
public:
    GLSLLangInitializer()
        : _languages{EShLangVertex, EShLangFragment}, _built_in_resource(DefaultTBuiltInResource) {
        glslang::InitializeProcess();
    }
    ~GLSLLangInitializer() {
        glslang::FinalizeProcess();
    }

    EShLanguage toShLanguage(VKUtil::ShaderType type) const {
        return _languages[type];
    }

    const TBuiltInResource& builtInResource() const {
        return _built_in_resource;
    }

private:
    EShLanguage _languages[VKUtil::SHADER_TYPE_COUNT];
    TBuiltInResource _built_in_resource;

};

}

VKUtil::VKUtil(const sp<ResourceManager>& resourceManager, const sp<RendererFactoryVulkan::Stub>& rendererFactory)
    : _ubo(sp<UBO>::make()), _resource_manager(resourceManager), _renderer_factory(rendererFactory), _graphics_context(nullptr, resourceManager)
{
}

VKUtil::~VKUtil()
{
    _renderer_factory->_device->waitIdle();

    _renderer_factory->_render_target->commandPool()->destroyCommandBuffers(_command_buffers.size(), _command_buffers.data());

#if defined(_DIRECT2DISPLAY)

#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    wl_shell_surface_destroy(shell_surface);
    wl_surface_destroy(surface);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (pointer)
        wl_pointer_destroy(pointer);
    wl_seat_destroy(seat);
    wl_shell_destroy(shell);
    wl_compositor_destroy(compositor);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    // todo : android cleanup (if required)
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(connection, window);
    xcb_disconnect(connection);
#endif
}

void VKUtil::initialize(GLContext& /*glContext*/)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);
    zoom = -2.5f;

    _uniforms = sp<VKBuffer>::make(_renderer_factory->_device, _resource_manager->recycler(), sp<Uploader::StandardLayout<UBO>>::make(_ubo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    updateUniformBuffers(_renderer_factory->_render_target);
    generateQuad(_renderer_factory->_device);

    const sp<Variable<bitmap>> tex = sp<Variable<bitmap>::Get>::make(_resource_manager->bitmapLoader(), "texture.png");

    const sp<VKTexture2D> texture = sp<VKTexture2D>::make(_resource_manager->recycler(), _renderer_factory->_render_target->commandPool(), tex);
    texture->upload(_graphics_context);
    _pipeline_factory = sp<PipelineFactoryVulkan>::make(_resource_manager, _renderer_factory);
    _pipeline_factory->_texture = texture;
    _pipeline_factory->_ubo = _uniforms;
    _pipeline = _pipeline_factory->build();

    buildCommandBuffers(_renderer_factory->_render_target);
    prepared = true;
}

void VKUtil::render()
{
    if (!prepared)
        return;
    draw(_renderer_factory->_render_target);
}

void VKUtil::updateUniformBuffers(const VKRenderTarget& renderTarget)
{
    _ubo->projection = glm::perspective(glm::radians(60.0f), (float)renderTarget.width() / (float)renderTarget.height(), 0.001f, 256.0f);
    glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, zoom));

    _ubo->model = viewMatrix * glm::translate(glm::mat4(1.0f), cameraPos);
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    _ubo->model = glm::rotate(_ubo->model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    _ubo->viewPos = glm::vec4(0.0f, 0.0f, -zoom, 0.0f);

    _uniforms->upload(_graphics_context);
}

void VKUtil::buildCommandBuffers(const VKRenderTarget& renderTarget)
{
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

    VkClearValue clearValues[2];
    clearValues[0].color = _background_color;
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = vks::initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderTarget.renderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = renderTarget.width();
    renderPassBeginInfo.renderArea.extent.height = renderTarget.height();
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    _command_buffers = renderTarget.makeCommandBuffers();
    for (size_t i = 0; i < _command_buffers.size(); ++i)
    {
        // Set target frame buffer
        renderPassBeginInfo.framebuffer = renderTarget.frameBuffers()[i];

        checkResult(vkBeginCommandBuffer(_command_buffers[i], &cmdBufInfo));

        vkCmdBeginRenderPass(_command_buffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = vks::initializers::viewport((float)renderTarget.width(), (float)renderTarget.height(), 0.0f, 1.0f);
        vkCmdSetViewport(_command_buffers[i], 0, 1, &viewport);

        VkRect2D scissor = vks::initializers::rect2D(renderTarget.width(), renderTarget.height(), 0, 0);
        vkCmdSetScissor(_command_buffers[i], 0, 1, &scissor);

        vkCmdBindDescriptorSets(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->vkPipelineLayout(), 0, 1, &_pipeline->vkDescriptorSet(), 0, nullptr);
        vkCmdBindPipeline(_command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->vkPipeline());

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers(_command_buffers[i], VERTEX_BUFFER_BIND_ID, 1, &_vertex_buffer->vkBuffer(), offsets);
        vkCmdBindIndexBuffer(_command_buffers[i], _index_buffer->vkBuffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdDrawIndexed(_command_buffers[i], indexCount, 1, 0, 0, 0);

        vkCmdEndRenderPass(_command_buffers[i]);

        checkResult(vkEndCommandBuffer(_command_buffers[i]));
    }
}

void VKUtil::generateQuad(const sp<VKDevice>& device)
{
    // Setup vertices for a single uv-mapped quad made from two triangles
    std::vector<Vertex> vertices =
    {
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
        { { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
        { {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
    };

    // Setup indices
    std::vector<uint32_t> indices = { 0,1,2, 2,3,0 };
    indexCount = static_cast<uint32_t>(indices.size());

    // Create buffers
    // For the sake of simplicity we won't stage the vertex data to the gpu memory
    _vertex_buffer = sp<VKBuffer>::make(device, _resource_manager->recycler(), sp<Uploader::Vector<Vertex>>::make(std::move(vertices)), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    _vertex_buffer->upload(_graphics_context);

    _index_buffer = sp<VKBuffer>::make(device, _resource_manager->recycler(), sp<Uploader::Vector<uint32_t>>::make(std::move(indices)), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    _index_buffer->upload(_graphics_context);
}

void VKUtil::draw(VKRenderTarget& renderTarget)
{
    uint32_t currentBuffer = renderTarget.acquire();
    renderTarget.submit(&_command_buffers[currentBuffer]);
    renderTarget.swap(currentBuffer);
}

void VKUtil::checkResult(VkResult result)
{
    DCHECK(result == VK_SUCCESS, "Vulkan error: %s", vks::tools::errorString(result).c_str());
}

VkPipelineShaderStageCreateInfo VKUtil::loadShaderSPIR(VkDevice device, std::string fileName, VkShaderStageFlagBits stage)
{
    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    shaderStage.module = vks::tools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device);
#else
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
#endif
    shaderStage.pName = "main";
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}

VkPipelineShaderStageCreateInfo VKUtil::loadShader(VkDevice device, const String& resid, ShaderType stage)
{
    const String content = Strings::loadFromReadable(Ark::instance().openAsset(resid));
    const std::vector<uint32_t> spirv = VKUtil::compileSPIR(content, stage);
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spirv.data();

    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    checkResult(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderStage.module));
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    shaderStage.pName = "main";
    return shaderStage;
}

std::vector<uint32_t> VKUtil::compileSPIR(const String& source, ShaderType shaderType)
{
    Global<GLSLLangInitializer> initializer;
    EShLanguage stage = initializer->toShLanguage(shaderType);
    glslang::TShader shader(stage);
    const char* sources[] = {source.c_str()};
    shader.setStrings(sources, 1);

    typedef std::map<uint32_t, uint32_t> TPerSetBaseBinding;
    std::array<std::array<uint32_t, EShLangCount>, glslang::EResCount> baseBinding;
    std::array<std::array<TPerSetBaseBinding, EShLangCount>, glslang::EResCount> baseBindingForSet;
    std::array<std::vector<std::string>, EShLangCount> baseResourceSetBinding;
    std::vector<std::pair<std::string, int32_t>> uniformLocationOverrides;

    int32_t uniformBase = 0;

    // Set IO mapper binding shift values
    for (int32_t r = 0; r < glslang::EResCount; ++r) {
        const glslang::TResourceType res = glslang::TResourceType(r);

        // Set base bindings
        shader.setShiftBinding(res, baseBinding[res][stage]);

        // Set bindings for particular resource sets
        // TODO: use a range based for loop here, when available in all environments.
        for (auto i = baseBindingForSet[res][stage].begin();
             i != baseBindingForSet[res][stage].end(); ++i)
            shader.setShiftBindingForSet(res, i->second, i->first);
    }
    shader.setFlattenUniformArrays(false);
    shader.setNoStorageFormat(false);
    shader.setResourceSetBinding(baseResourceSetBinding[stage]);

    shader.setUniformLocationBase(uniformBase);

    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    if (!shader.parse(&initializer->builtInResource(), 110, false, EShMsgDefault))
        DFATAL("Compile error: %s", shader.getInfoLog());
    {
        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
            DFATAL("Link error: %s", shader.getInfoLog());

        if (program.getIntermediate(stage)) {
            std::vector<uint32_t> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            spvOptions.disableOptimizer = false;
            spvOptions.optimizeSize = false;
            spvOptions.disassemble = false;
            spvOptions.validate = true;
            glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger, &spvOptions);
            return spirv;
        }
    }
    return {};
}

}
}
