#include "renderer/util/render_util.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "core/types/global.h"
#include "core/types/shared_ptr.h"
#include "core/util/uploader_type.h"

#include "graphics/base/rect.h"
#include "graphics/base/v3.h"

#include "renderer/base/model.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/impl/vertices/vertices_nine_patch_quads.h"
#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/impl/vertices/vertices_quad.h"


namespace ark {

namespace {

constexpr TBuiltInResource DefaultTBuiltInResource = {
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
/* TODO: They are new in Vulkan 1.3
    maxMeshOutputVerticesEXT					= 2048;
    maxMeshOutputPrimitivesEXT					= 2048;
    maxMeshWorkGroupSizeX_EXT					= 256;
    maxMeshWorkGroupSizeY_EXT					= 256;
    maxMeshWorkGroupSizeZ_EXT					= 256;
    maxTaskWorkGroupSizeX_EXT					= 256;
    maxTaskWorkGroupSizeY_EXT					= 256;
    maxTaskWorkGroupSizeZ_EXT					= 256;
    maxMeshViewCountEXT						= 4;
*/
    /* .maxDualSourceDrawBuffersEXT = */ 1,
};

constexpr TLimits DefaultTBuiltInResourceLimits = {
    /* .nonInductiveForLoops = */ true,
    /* .whileLoops = */ true,
    /* .doWhileLoops = */ true,
    /* .generalUniformIndexing = */ true,
    /* .generalAttributeMatrixVectorIndexing = */ true,
    /* .generalVaryingIndexing = */ true,
    /* .generalSamplerIndexing = */ true,
    /* .generalVariableIndexing = */ true,
    /* .generalConstantMatrixVectorIndexing = */ true
};

class GLSLLangInitializer {
public:
    GLSLLangInitializer()
#ifndef ANDROID
        : _languages{EShLangVertex, EShLangTessControl, EShLangTessEvaluation, EShLangGeometry, EShLangFragment, EShLangCompute},
#else
        : _languages{EShLangVertex, EShLangFragment, EShLangCompute},
#endif
          _built_in_resource(DefaultTBuiltInResource) {
        _built_in_resource.limits = DefaultTBuiltInResourceLimits;
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

void setLayoutQualifierBinding(const ShaderPreprocessor::Declaration& declaration, const String& qualifierName, uint32_t binding)
{
    StringBuffer sb;
    sb << "layout (" << qualifierName << " = " << binding << ") " << declaration.source();
    declaration.setSource(sb.str());
}

uint32_t getNextLayoutLocation(const ShaderPreprocessor::Declaration& declar, uint32_t& counter)
{
    const uint32_t location = counter;
    //TODO: Consider merge uniform type and attribute type into one, something like ShaderDataType.
    Uniform::Type type;
    if(declar.type() == "vec4b")
        type = Uniform::TYPE_I4;
    else if(declar.type() == "vec3b")
        type = Uniform::TYPE_I3;
    else
        type = Uniform::toType(declar.type());
    switch(type)
    {
        case Uniform::TYPE_MAT3:
        case Uniform::TYPE_MAT3V:
            counter += 3 * declar.length();
        break;
        case Uniform::TYPE_MAT4:
        case Uniform::TYPE_MAT4V:
            counter += 4 * declar.length();
        break;
        default:
            counter ++;
    }
    return location;
}

}

bytearray RenderUtil::makeUnitCubeVertices(bool flipWindingOrder)
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f
    };

    if(flipWindingOrder)
    {
        sp<ByteArray::Fixed<sizeof(vertices)>> flipped = sp<ByteArray::Fixed<sizeof(vertices)>>::make();
        memcpy(flipped->buf(), vertices, flipped->size());
        V3* buf = reinterpret_cast<V3*>(flipped->buf());
        for(uint32_t i = 0; i < 6; ++i, buf += 4)
            std::swap(buf[1], buf[2]);
        return flipped;
    }

    return sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Attribute RenderUtil::makePredefinedAttribute(const String& name, const String& type, Attribute::Usage layoutType)
{
    if(type == "vec3" || type == "v3f")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 3, false};
    if(type == "vec2" || type == "v2f")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 2, false};
    if(type == "float")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 1, false};
    if(type == "int" || type == "uint")
        return {layoutType, name, Attribute::TYPE_INTEGER, type, 1, false};
    if(type == "vec4" || type == "v4f")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 4, false};
    if(type == "vec4b" || type == "v4b")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 4, true};
    if(type == "vec3b" || type == "v3fb")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 3, true};
    if(type == "uint8")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 1, false};
    if(type == "mat4")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 16, false};
    if(type == "ivec4")
        return {layoutType, name, Attribute::TYPE_INTEGER, type, 4, false};
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return {};
}

uint32_t RenderUtil::hash(const element_index_t* buf, size_t len)
{
    uint32_t h = 0;
    for(size_t i = 0; i < len; ++i)
        h = h * 101 + buf[i];
    return h;
}

String RenderUtil::outAttributeName(const String& name, PipelineInput::ShaderStage preStage)
{
    DCHECK(preStage == PipelineInput::SHADER_STAGE_NONE || preStage == PipelineInput::SHADER_STAGE_VERTEX, "Only none and vertex stage's out attribute name supported");
    const char sPrefix[][8] = {"a_", "v_"};
    const String prefix = sPrefix[preStage + 1];
    return name.startsWith(prefix) ? name : prefix + Strings::capitalizeFirst(name);
}

uint32_t RenderUtil::getChannelSize(Texture::Format format)
{
    return (format & Texture::FORMAT_RGBA) + 1;
}

uint32_t RenderUtil::getPixelSize(Texture::Format format)
{
    if(format == Texture::FORMAT_AUTO)
        return 4;

    return (static_cast<uint32_t>(format & Texture::FORMAT_RGBA) + 1) * getComponentSize(format);
}

uint32_t RenderUtil::getComponentSize(Texture::Format format)
{
    const uint32_t componentFormat = format & Texture::FORMAT_32_BIT;
    if(componentFormat == Texture::FORMAT_8_BIT || componentFormat == 0)
        return 1;
    if(componentFormat == Texture::FORMAT_16_BIT)
        return 2;
    CHECK((format & Texture::FORMAT_32_BIT) == Texture::FORMAT_32_BIT, "Unknown component format %d", format);
    return 4;
}

std::vector<uint32_t> RenderUtil::compileSPIR(const String& source, PipelineInput::ShaderStage stage, Ark::RendererTarget renderTarget)
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
        const auto res = static_cast<glslang::TResourceType>(r);
        // Set base bindings
        shader.setShiftBinding(res, baseBinding[res][esStage]);
        // Set bindings for particular resource sets
        for(const auto& [k, v] : baseBindingForSet[res][esStage])
            shader.setShiftBindingForSet(res, v, k);
    }
#ifdef ENABLE_HLSL
    shader.setFlattenUniformArrays(false);
#endif
    shader.setNoStorageFormat(false);
    shader.setResourceSetBinding(baseResourceSetBinding[esStage]);

    shader.setUniformLocationBase(uniformBase);
//TODO: Use the specified version
    switch(renderTarget)
    {
        case Ark::RENDERER_TARGET_AUTO:
        case Ark::RENDERER_TARGET_OPENGL:
            shader.setEnvInput(glslang::EShSourceGlsl, esStage, glslang::EShClientOpenGL, 450);
            shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
            break;
        case Ark::RENDERER_TARGET_VULKAN:
            shader.setEnvInput(glslang::EShSourceGlsl, esStage, glslang::EShClientVulkan, 100);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
            break;
    }
#ifdef ARK_PLATFORM_DARWIN
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
#else
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
#endif

    if(!shader.parse(&initializer->builtInResource(), 100, false, EShMsgDefault))
        FATAL("Compile error:\n%s\n\n%s", source.c_str(), shader.getInfoLog());
    {
        glslang::TProgram program;
        program.addShader(&shader);
        if(!program.link(EShMsgDefault))
            FATAL("Link error: %s\n\n%s", source.c_str(), shader.getInfoLog());

        if(const glslang::TIntermediate* intermedia = program.getIntermediate(esStage)) {
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

std::vector<ShaderPreprocessor::Declaration> RenderUtil::setupLayoutLocation(const PipelineBuildingContext& context, const ShaderPreprocessor::DeclarationList& declarations)
{
    std::vector<ShaderPreprocessor::Declaration> locations;
    std::map<uint32_t, std::vector<const ShaderPreprocessor::Declaration*>> divisors;

    for(const ShaderPreprocessor::Declaration& i : declarations.vars().values()) {
        const auto iter = context._attributes.find(i.name());
        DCHECK(iter != context._attributes.end(), "Cannot find attribute %s", i.name().c_str());
        const Attribute& attribute = iter->second;
        divisors[attribute.divisor()].push_back(&i);
    }

    for(const auto& i : divisors)
    {
        for(const ShaderPreprocessor::Declaration* j : i.second)
            locations.push_back(*j);
    }

    return locations;
}

uint32_t RenderUtil::setLayoutDescriptor(const std::vector<ShaderPreprocessor::Declaration>& declarations, const String& qualifierName, uint32_t start)
{
    uint32_t counter = start;
    for(const ShaderPreprocessor::Declaration& i : declarations)
        setLayoutQualifierBinding(i, qualifierName, getNextLayoutLocation(i, counter));
    return counter;
}

uint32_t RenderUtil::setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& declarations, const String& descriptor, uint32_t start)
{
    return setLayoutDescriptor(declarations.vars().values(), descriptor, start);
}

uint32_t RenderUtil::setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& ins, const ShaderPreprocessor::DeclarationList& outs, const String& qualifierName, uint32_t start)
{
    uint32_t counter = start;
    DCHECK_WARN(ins.vars().size() == outs.vars().size(), "Output/Input mismatch, output and input have different numbers of items: [%s] vs [%s]",
                Strings::join(&ins.vars().keys().at(0), 0, ins.vars().keys().size()).c_str(), Strings::join(&outs.vars().keys().at(0), 0, outs.vars().keys().size()).c_str());
    for(const ShaderPreprocessor::Declaration& i : ins.vars().values()) {
        uint32_t binding = getNextLayoutLocation(i, counter);
        setLayoutQualifierBinding(i, qualifierName, binding);
        const String outName = Strings::capitalizeFirst(i.name().startsWith("v_") ? i.name().substr(2) : i.name());
        bool hasOutName = outs.vars().has(outName);
        DCHECK_WARN(hasOutName, "Output/Input mismatch, \"%s\" exists in input but not found in next stage of shader", outName.c_str());
        if(hasOutName)
            setLayoutQualifierBinding(outs.vars().at(outName), qualifierName, binding);
    }
    return counter;
}

}
