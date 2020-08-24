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
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/uploader.h"

#include "renderer/vulkan/base/vk_buffer.h"
#include "renderer/vulkan/base/vk_command_pool.h"
#include "renderer/vulkan/base/vk_command_buffers.h"
#include "renderer/vulkan/base/vk_device.h"
#include "renderer/vulkan/base/vk_instance.h"
#include "renderer/vulkan/base/vk_pipeline.h"
#include "renderer/vulkan/base/vk_renderer.h"
#include "renderer/vulkan/base/vk_render_target.h"
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
#ifndef ANDROID
        : _languages{EShLangVertex, EShLangTessControl, EShLangTessEvaluation, EShLangGeometry, EShLangFragment, EShLangCompute},
#else
        : _languages{EShLangVertex, EShLangFragment, EShLangCompute},
#endif
          _built_in_resource(DefaultTBuiltInResource) {
        glslang::InitializeProcess();
    }
    ~GLSLLangInitializer() {
        glslang::FinalizeProcess();
    }

    EShLanguage toShLanguage(PipelineInput::ShaderStage stage) const {
        DCHECK(stage > PipelineInput::SHADER_STAGE_NONE && stage < PipelineInput::SHADER_STAGE_COUNT, "Illegal PipelineInput::ShaderStage: %d", stage);
        return _languages[stage];
    }

    const TBuiltInResource& builtInResource() const {
        return _built_in_resource;
    }

private:
    EShLanguage _languages[PipelineInput::SHADER_STAGE_COUNT];
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
    shaderStage.module = vks::tools::loadShader(fileName.c_str(), device);
    shaderStage.pName = "main";
    DASSERT(shaderStage.module != VK_NULL_HANDLE);
    return shaderStage;
}

VkPipelineShaderStageCreateInfo VKUtil::loadShader(VkDevice device, const String& resid, PipelineInput::ShaderStage stage)
{
    const String content = Strings::loadFromReadable(Ark::instance().openAsset(resid));
    return createShader(device, content, stage);
}

VkPipelineShaderStageCreateInfo VKUtil::createShader(VkDevice device, const String& source, PipelineInput::ShaderStage stage)
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

void VKUtil::createImage(const VKDevice& device, const VkImageCreateInfo& imageCreateInfo, VkImage* image, VkDeviceMemory* memory, VkMemoryPropertyFlags propertyFlags)
{
    VkDevice logicalDevice = device.vkLogicalDevice();
    checkResult(vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, image));

    VkMemoryRequirements memReqs = {};
    VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
    vkGetImageMemoryRequirements(logicalDevice, *image, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, propertyFlags);
    checkResult(vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, memory));
    checkResult(vkBindImageMemory(logicalDevice, *image, *memory, 0));
}

VkFormat VKUtil::toAttributeFormat(Attribute::Type type, uint32_t length)
{
    if(length > 0 && length < 5)
    {
        switch(type)
        {
            case Attribute::TYPE_FLOAT:
            {
                const VkFormat formats[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
                return formats[length - 1];
            }
            case Attribute::TYPE_BYTE:
            case Attribute::TYPE_UBYTE:
            {
                const VkFormat formats[4] = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8A8_UNORM};
                return formats[length - 1];
            }
            case Attribute::TYPE_INTEGER:
            {
                const VkFormat formats[4] = {VK_FORMAT_R32_SINT, VK_FORMAT_R32G32_SINT, VK_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32A32_SINT};
                return formats[length - 1];
            }
            case Attribute::TYPE_SHORT:
            {
                const VkFormat formats[4] = {VK_FORMAT_R16_SNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16A16_SNORM};
                return formats[length - 1];
            }
            case Attribute::TYPE_USHORT:
            {
                const VkFormat formats[4] = {VK_FORMAT_R16_UNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16A16_UNORM};
                return formats[length - 1];
            }
            default:
                break;
        }
    }
    DFATAL("Unsupport type %d, length %d", type, length);
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

VkFormat VKUtil::toTextureFormat(uint32_t componentSize, uint8_t channels, Texture::Format format)
{
    static const VkFormat vkFormats[] = {VK_FORMAT_R8_UNORM, VK_FORMAT_R8_SNORM, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SFLOAT, VK_FORMAT_R16_UNORM, VK_FORMAT_R16_SNORM, VK_FORMAT_R32_UINT, VK_FORMAT_R32_SINT,
                                         VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_SNORM, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_SNORM, VK_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_SINT,
                                         VK_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_SNORM, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_SNORM, VK_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_SINT,
                                         VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_SINT};
    DCHECK(!(format & Texture::FORMAT_SIGNED && format & Texture::FORMAT_F16), "FORMAT_SIGNED format can not combined with FORMAT_F16");
    DCHECK(!(format & Texture::FORMAT_SIGNED && format & Texture::FORMAT_F32), "FORMAT_SIGNED format can not combined with FORMAT_F32");
    uint32_t signedOffset = (format & Texture::FORMAT_SIGNED) == Texture::FORMAT_SIGNED ? 1 : 0;
    uint32_t floatPointOffset = (format & Texture::FORMAT_F16) ? 1 : ((format & Texture::FORMAT_F32) ? 1 : 0);
    uint32_t channel8 = (channels - 1) * 8;
    DCHECK(componentSize > 0 && componentSize <= 4 && componentSize != 3, "Unsupported color-depth: %d", componentSize * 8);
    return static_cast<VkFormat>(vkFormats[channel8 + (componentSize - 1) * 2 + floatPointOffset] + signedOffset);
}

VkFormat VKUtil::toTextureFormat(const Bitmap& bitmap, Texture::Format format)
{
    return toTextureFormat(bitmap.rowBytes() / bitmap.width() / bitmap.channels(), bitmap.channels(), format);
}

VkFormat VKUtil::toTextureFormat(Texture::Format format)
{
    DCHECK(format != Texture::FORMAT_AUTO, "Cannot determine texture format(auto) without a bitmap");
    return toTextureFormat((format & Texture::FORMAT_F16) ? 2 : (format & Texture::FORMAT_F32 ? 4 : 1), (format & Texture::FORMAT_RGBA) + 1, format);
}

VkFrontFace VKUtil::toFrontFace(PipelineBindings::FrontFace frontFace)
{
    switch(frontFace) {
        case PipelineBindings::FRONT_FACE_DEFAULT:
        case PipelineBindings::FRONT_FACE_COUTER_CLOCK_WISE:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case PipelineBindings::FRONT_FACE_CLOCK_WISE:
            return VK_FRONT_FACE_CLOCKWISE;
        default:
            DFATAL("Unknow front face: %d", frontFace);
    }
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

VkCompareOp VKUtil::toCompareOp(PipelineBindings::CompareFunc func)
{
    const VkCompareOp compareOps[] = {VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_ALWAYS, VK_COMPARE_OP_NEVER, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_LESS,
                                      VK_COMPARE_OP_GREATER, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL};
    DASSERT(func < PipelineBindings::COMPARE_FUNC_LENGTH);
    return compareOps[func];
}

VkStencilOp VKUtil::toStencilOp(PipelineBindings::StencilFunc func)
{
    const VkStencilOp stencilOps[] = {VK_STENCIL_OP_KEEP, VK_STENCIL_OP_ZERO, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_INCREMENT_AND_WRAP,
                                      VK_STENCIL_OP_DECREMENT_AND_CLAMP, VK_STENCIL_OP_DECREMENT_AND_WRAP, VK_STENCIL_OP_INVERT};
    DASSERT(func < PipelineBindings::STENCIL_FUNC_LENGTH);
    return stencilOps[func];
}

VkImageUsageFlags VKUtil::toTextureUsage(Texture::Usage usage)
{
    VkImageUsageFlags vkFlags = 0;
    if(usage == Texture::USAGE_COLOR_ATTACHMENT)
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(usage & Texture::USAGE_DEPTH_STENCIL_ATTACHMENT)
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    return vkFlags;
}

VkImageAspectFlags VKUtil::toTextureAspect(Texture::Usage usage)
{
    VkImageAspectFlags vkFlags = 0;
    if(usage == Texture::USAGE_COLOR_ATTACHMENT)
        vkFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if(usage & Texture::USAGE_DEPTH_ATTACHMENT)
        vkFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    if(usage & Texture::USAGE_STENCIL_ATTACHMENT)
        vkFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    return vkFlags;
}

VkShaderStageFlagBits VKUtil::toStage(PipelineInput::ShaderStage stage)
{
#ifndef ANDROID
    static const VkShaderStageFlagBits vkStages[PipelineInput::SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                                               VK_SHADER_STAGE_GEOMETRY_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#else
    static const VkShaderStageFlagBits vkStages[PipelineInput::SHADER_STAGE_COUNT] = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_COMPUTE_BIT};
#endif
    DCHECK(stage > PipelineInput::SHADER_STAGE_NONE && stage < PipelineInput::SHADER_STAGE_COUNT, "Illegal PipelineInput::ShaderStage: %d", stage);
    return vkStages[stage];
}

VkPrimitiveTopology VKUtil::toPrimitiveTopology(ModelLoader::RenderMode mode)
{
    static const VkPrimitiveTopology topologies[ModelLoader::RENDER_MODE_COUNT] = {VK_PRIMITIVE_TOPOLOGY_LINE_LIST, VK_PRIMITIVE_TOPOLOGY_POINT_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP};
    DCHECK(mode >= 0 && mode < ModelLoader::RENDER_MODE_COUNT, "Unsupported render-mode: %d", mode);
    return topologies[mode];
}

std::vector<uint32_t> VKUtil::compileSPIR(const String& source, PipelineInput::ShaderStage stage)
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
