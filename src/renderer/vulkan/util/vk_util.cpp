#include "renderer/vulkan/util/vk_util.h"

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

#include "renderer/base/attribute.h"
#include "renderer/base/render_context.h"
#include "renderer/base/resource_manager.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
#include "renderer/vulkan/base/vk_texture_2d.h"
#include "renderer/vulkan/pipeline_factory/pipeline_factory_vulkan.h"
#include "renderer/vulkan/util/vulkan_tools.h"
#include "renderer/vulkan/util/vulkan_debug.h"

#include "generated/vulkan_plugin.h"

#define VERTEX_BUFFER_BIND_ID 0

namespace ark {

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

    EShLanguage toShLanguage(Shader::Stage stage) const {
        DCHECK(stage > Shader::STAGE_NONE && stage < Shader::STAGE_COUNT, "Illegal Shader::Stage: %d", stage);
        return _languages[stage];
    }

    const TBuiltInResource& builtInResource() const {
        return _built_in_resource;
    }

private:
    EShLanguage _languages[Shader::STAGE_COUNT];
    TBuiltInResource _built_in_resource;

};

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

VkPipelineShaderStageCreateInfo VKUtil::loadShader(VkDevice device, const String& resid, Shader::Stage stage)
{
    const String content = Strings::loadFromReadable(Ark::instance().openAsset(resid));
    return createShader(device, content, stage);
}

VkPipelineShaderStageCreateInfo VKUtil::createShader(VkDevice device, const String& source, Shader::Stage stage)
{
    const std::vector<uint32_t> spirv = VKUtil::compileSPIR(source, stage);
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = spirv.size() * sizeof(uint32_t);
    moduleCreateInfo.pCode = spirv.data();

    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = toStage(stage);
    checkResult(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderStage.module));
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    shaderStage.pName = "main";
    return shaderStage;
}

VkFormat VKUtil::getAttributeFormat(const Attribute& attribute)
{
    if(attribute.type() == Attribute::TYPE_FLOAT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_INTEGER)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_SHORT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16A16_SNORM};
            return formats[attribute.length() - 1];
        }
    }
    else if(attribute.type() == Attribute::TYPE_USHORT)
    {
        if(attribute.length() < 5)
        {
            const VkFormat formats[4] = {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM};
            return formats[attribute.length() - 1];
        }
    }
    DFATAL("Unsupport type %s, length %d", attribute.declareType().c_str(), attribute.length());
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

VkFormat VKUtil::toTextureFormat(const Bitmap& bitmap, Texture::Format format)
{
    static const VkFormat vkFormats[] = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT,
                                         VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT,
                                         VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT,
                                         VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT};
    uint32_t signedOffset = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED ? 1 : 0;
    uint32_t byteCount = bitmap.rowBytes() / bitmap.width() / bitmap.channels();
    uint32_t channel8 = (bitmap.channels() - 1) * 8;
    DCHECK(byteCount > 0 && byteCount <= 4 && byteCount != 3, "Unsupported color-depth: %d", byteCount * 8);
    return vkFormats[channel8 + (byteCount - 1) * 2 + signedOffset];
}

VkShaderStageFlagBits VKUtil::toStage(Shader::Stage stage)
{
    static const VkShaderStageFlagBits vkStages[] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
    DCHECK(stage > Shader::STAGE_NONE && stage < Shader::STAGE_COUNT, "Illegal Shader::Stage: %d", stage);
    return vkStages[stage];
}

VkPrimitiveTopology VKUtil::toPrimitiveTopology(RenderModel::Mode mode)
{
    static const VkPrimitiveTopology topologies[RenderModel::RENDER_MODE_COUNT] = {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP};
    DCHECK(mode >= 0 && mode < RenderModel::RENDER_MODE_COUNT, "Unsupported render-mode: %d", mode);
    return topologies[mode];
}

std::vector<uint32_t> VKUtil::compileSPIR(const String& source, Shader::Stage stage)
{
    const Global<GLSLLangInitializer> initializer;
    EShLanguage esStage = initializer->toShLanguage(stage);
    glslang::TShader shader(esStage);
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
        shader.setShiftBinding(res, baseBinding[res][esStage]);

        // Set bindings for particular resource sets
        // TODO: use a range based for loop here, when available in all environments.
        for (auto i = baseBindingForSet[res][esStage].begin();
             i != baseBindingForSet[res][esStage].end(); ++i)
            shader.setShiftBindingForSet(res, i->second, i->first);
    }
    shader.setFlattenUniformArrays(false);
    shader.setNoStorageFormat(false);
    shader.setResourceSetBinding(baseResourceSetBinding[esStage]);

    shader.setUniformLocationBase(uniformBase);

    shader.setEnvInput(glslang::EShSourceGlsl, esStage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    if (!shader.parse(&initializer->builtInResource(), 100, false, EShMsgDefault))
        DFATAL("Compile error: %s\n\n%s", source.c_str(), shader.getInfoLog());
    {
        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(EShMsgDefault))
            DFATAL("Link error: %s\n\n%s", source.c_str(), shader.getInfoLog());

        glslang::TIntermediate* intermedia = program.getIntermediate(esStage);
        if (intermedia) {
            std::vector<uint32_t> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
            spvOptions.disableOptimizer = false;
            spvOptions.optimizeSize = false;
            spvOptions.disassemble = false;
            spvOptions.validate = true;
            glslang::GlslangToSpv(*intermedia, spirv, &logger, &spvOptions);
            return spirv;
        }
    }
    return {};
}

}
}
