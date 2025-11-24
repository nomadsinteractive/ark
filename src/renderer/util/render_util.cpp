#include "renderer/util/render_util.h"

#include <ranges>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include "core/types/global.h"
#include "core/types/shared_ptr.h"
#include "core/util/uploader_type.h"

#include "renderer/base/model.h"
#include "renderer/base/pipeline_building_context.h"


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

    EShLanguage toShLanguage(enums::ShaderStageBit stage) const {
        DCHECK(stage > enums::SHADER_STAGE_BIT_NONE && stage < enums::SHADER_STAGE_BIT_COUNT, "Illegal ShaderStage::BitSet: %d", stage);
        return _languages[stage];
    }

    const TBuiltInResource& builtInResource() const {
        return _built_in_resource;
    }

private:
    EShLanguage _languages[enums::SHADER_STAGE_BIT_COUNT];
    TBuiltInResource _built_in_resource;
};

void setLayoutQualifierBinding(const ShaderPreprocessor::Declaration& declaration, const String& qualifierName, const uint32_t binding, const int32_t set = -1)
{
    StringBuffer sb;
    sb << "layout (";
    if(set >= 0)
        sb << "set = " << set << ", ";
    sb << qualifierName << " = " << binding << ") " << declaration.source();
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

glslang::EShTargetLanguageVersion toTargetLanguageVersion(const uint32_t targetLanguageVersion, const glslang::EShTargetLanguageVersion defaultVersion)
{
    switch(targetLanguageVersion)
    {
        case 10:
            return glslang::EShTargetSpv_1_0;
        case 11:
            return glslang::EShTargetSpv_1_1;
        case 12:
            return glslang::EShTargetSpv_1_2;
        case 13:
            return glslang::EShTargetSpv_1_3;
        case 14:
            return glslang::EShTargetSpv_1_4;
        case 15:
            return glslang::EShTargetSpv_1_5;
        case 16:
            return glslang::EShTargetSpv_1_6;
        default:
        break;
    }
    return defaultVersion;
}

String addSourceLineNumbers(const String& source)
{
    StringBuffer sb;
    uint32_t lineNumber = 0;
    source.split('\n', true, [&sb, &lineNumber](const String& line) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%-6u ", ++lineNumber);
        sb << buf << line << std::endl;
        return true;
    });
    return sb.str();
}

}

bytearray RenderUtil::makeUnitCubeVertices(const bool flipWindingOrder)
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
        auto* buf = reinterpret_cast<V3*>(flipped->buf());
        for(uint32_t i = 0; i < 6; ++i, buf += 4)
            std::swap(buf[1], buf[2]);
        return flipped;
    }

    return sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Attribute RenderUtil::makePredefinedAttribute(const String& name, const String& type, Attribute::Usage layoutType)
{
    if(type == "vec3")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 3, false};
    if(type == "vec2")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 2, false};
    if(type == "float")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 1, false};
    if(type == "int")
        return {layoutType, name, Attribute::TYPE_INT, type, 1, false};
    if(type == "uint")
        return {layoutType, name, Attribute::TYPE_UINT, type, 1, false};
    if(type == "vec4")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 4, false};
    if(type == "vec4b")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 4, true};
    if(type == "vec3b")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 3, true};
    if(type == "uint8")
        return {layoutType, name, Attribute::TYPE_UBYTE, type, 1, false};
    if(type == "mat4")
        return {layoutType, name, Attribute::TYPE_FLOAT, type, 16, false};
    if(type == "ivec2")
        return {layoutType, name, Attribute::TYPE_INT, type, 2, false};
    if(type == "ivec4")
        return {layoutType, name, Attribute::TYPE_INT, type, 4, false};
    if(type == "uvec2")
        return {layoutType, name, Attribute::TYPE_UINT, type, 2, false};
    if(type == "uvec4")
        return {layoutType, name, Attribute::TYPE_UINT, type, 4, false};
    DFATAL("Unknown attribute type \"%s\"", type.c_str());
    return {};
}

uint32_t RenderUtil::hash(const element_index_t* buf, const size_t len)
{
    uint32_t h = 0;
    for(size_t i = 0; i < len; ++i)
        h = h * 101 + buf[i];
    return h;
}

Attribute::Usage RenderUtil::toAttributeLayoutType(const String& name, const String& type)
{
    const String n = name.toLower();
    if(n.startsWith("uv") || n.startsWith("texcoordinate"))
    {
        CHECK(type == "int" || type == "vec2" || type == "vec3", "Unacceptable TexCoordinate type: '%s', must be in [int, vec2, vec3]", type.c_str());
        return Attribute::USAGE_TEX_COORD;
    }
    if(n.startsWith("position"))
    {
        CHECK(type == "int" || type == "vec2" || type == "vec3" || type == "vec4", "Unacceptable Position type: '%s', must be in [int, vec2, vec3, vec4]", type.c_str());
        return Attribute::USAGE_POSITION;
    }
    if(n.startsWith("color"))
    {
        CHECK(type == "int" || type == "vec3" || type == "vec4"|| type == "vec3b" || type == "vec4b", "Unacceptable Color type: '%s', must be in [int, vec3, vec4, vec3b, vec4b]", type.c_str());
        return Attribute::USAGE_COLOR;
    }
    if(n == "model")
    {
        CHECK(type == "mat4", "Unacceptable Model type: '%s', must be in [mat4]", type.c_str());
        return Attribute::USAGE_MODEL_MATRIX;
    }
    if(n == "normal")
    {
        CHECK(type == "vec3" || type == "vec4", "Unacceptable Normal type: '%s', must be in [vec3, vec4]", type.c_str());
        return Attribute::USAGE_NORMAL;
    }
    if(n == "tangent")
    {
        CHECK(type == "vec3" || type == "vec4", "Unacceptable Tangent type: '%s', must be in [vec3, vec4]", type.c_str());
        return Attribute::USAGE_TANGENT;
    }
    if(n == "bitangent")
    {
        CHECK(type == "vec3" || type == "vec4", "Unacceptable Bitangent type: '%s', must be in [vec3, vec4]", type.c_str());
        return Attribute::USAGE_BITANGENT;
    }
    if(n == "materialid")
    {
        CHECK(type == "int" || type == "uint", "Unacceptable MaterialId type: '%s', must be in [int, uint]", type.c_str());
        return Attribute::USAGE_MATERIAL_ID;
    }
    if(n == "nodeid")
    {
        CHECK(type == "int" || type == "uint", "Unacceptable NodeId type: '%s', must be in [int, uint]", type.c_str());
        return Attribute::USAGE_NODE_ID;
    }
    return Attribute::USAGE_CUSTOM;
}

String RenderUtil::outAttributeName(const String& name, enums::ShaderStageBit preStage)
{
    DCHECK(preStage == enums::SHADER_STAGE_BIT_NONE || preStage == enums::SHADER_STAGE_BIT_VERTEX, "Only none and vertex stage's out attribute name supported");
    constexpr char sPrefix[][8] = {"a_", "v_"};
    const String prefix = sPrefix[preStage + 1];
    return name.startsWith(prefix) ? name : prefix + Strings::capitalizeFirst(name);
}

uint32_t RenderUtil::getChannelSize(Texture::Format format)
{
    DASSERT(format != Texture::FORMAT_AUTO);
    return (format & Texture::FORMAT_RGBA).bits() + 1;
}

uint32_t RenderUtil::getPixelSize(Texture::Format format)
{
    if(format == Texture::FORMAT_AUTO)
        return 4;

    return ((format & Texture::FORMAT_RGBA).bits() + 1) * getComponentSize(format);
}

uint32_t RenderUtil::getComponentSize(const Texture::Format format)
{
    const uint32_t componentFormat = (format & Texture::FORMAT_BIT_MASK).bits();
    if(componentFormat == Texture::FORMAT_8_BIT || componentFormat == 0)
        return 1;
    if(componentFormat == Texture::FORMAT_16_BIT)
        return 2;
    if(componentFormat == Texture::FORMAT_24_BIT)
        return 3;
    CHECK(componentFormat == Texture::FORMAT_32_BIT, "Unknown component format %d", format);
    return 4;
}

std::pair<int32_t, int32_t> RenderUtil::getRenderTargetResolution(const RenderTarget::Configure& configure)
{
    CHECK(!configure._color_attachments.empty() || configure._depth_stencil_attachment, "RenderTarget should have at least one attachment");
    const sp<Texture>& attachment = configure._color_attachments.empty() ? configure._depth_stencil_attachment : configure._color_attachments.at(0)._texture;
    return {attachment->width(), attachment->height()};
}

Vector<uint32_t> RenderUtil::compileSPIR(const String& source, enums::ShaderStageBit stage, enums::RenderingBackendBit renderTarget, const uint32_t targetLanguageVersion)
{
    const Global<GLSLLangInitializer> initializer;
    const EShLanguage esStage = initializer->toShLanguage(stage);
    glslang::TShader shader(esStage);
    const char* sources[] = {source.c_str()};
    shader.setStrings(sources, 1);

    typedef Map<uint32_t, uint32_t> TPerSetBaseBinding;
    std::array<std::array<uint32_t, EShLangCount>, glslang::EResCount> baseBinding;
    std::array<std::array<TPerSetBaseBinding, EShLangCount>, glslang::EResCount> baseBindingForSet;
    std::array<Vector<std::string>, EShLangCount> baseResourceSetBinding;
    Vector<std::pair<std::string, int32_t>> uniformLocationOverrides;

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
        case enums::RENDERING_BACKEND_BIT_OPENGL:
            shader.setEnvInput(glslang::EShSourceGlsl, esStage, glslang::EShClientOpenGL, 450);
            shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
            break;
        case enums::RENDERING_BACKEND_BIT_VULKAN:
            shader.setEnvInput(glslang::EShSourceGlsl, esStage, glslang::EShClientVulkan, 100);
            shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_2);
            break;
        default:
            FATAL("Unsupported");
            break;
    }
#ifdef ARK_PLATFORM_DARWIN
    shader.setEnvTarget(glslang::EShTargetSpv, toTargetLanguageVersion(targetLanguageVersion, glslang::EShTargetSpv_1_5));
#else
    shader.setEnvTarget(glslang::EShTargetSpv, toTargetLanguageVersion(targetLanguageVersion, glslang::EShTargetSpv_1_5));
#endif

    if(!shader.parse(&initializer->builtInResource(), 100, false, EShMsgDefault))
        FATAL("Compile error:\n%s\n\n%s", addSourceLineNumbers(source).c_str(), shader.getInfoLog());
    {
        glslang::TProgram program;
        program.addShader(&shader);
        if(!program.link(EShMsgDefault))
            FATAL("Link error: %s\n\n%s", addSourceLineNumbers(source).c_str(), shader.getInfoLog());

        if(const glslang::TIntermediate* intermedia = program.getIntermediate(esStage))
        {
            Vector<uint32_t> spirv;
            spv::SpvBuildLogger logger;
            glslang::SpvOptions spvOptions;
#ifdef ARK_FLAG_BUILD_TYPE
            spvOptions.disableOptimizer = false;
            spvOptions.optimizeSize = false;
            spvOptions.disassemble = false;
            spvOptions.validate = true;
#endif
            glslang::GlslangToSpv(*intermedia, spirv, &logger, &spvOptions);
            return spirv;
        }
    }
    return {};
}

Vector<ShaderPreprocessor::Declaration> RenderUtil::setupLayoutLocation(const PipelineBuildingContext& context, const ShaderPreprocessor::DeclarationList& declarations)
{
    Vector<ShaderPreprocessor::Declaration> locations;
    Map<uint32_t, Vector<const ShaderPreprocessor::Declaration*>> divisors;

    Vector<ShaderPreprocessor::Declaration> declares = declarations.vars().values();
    std::stable_sort(declares.begin(), declares.end());

    for(const ShaderPreprocessor::Declaration& i : declares) {
        const auto iter = context._attributes.find(i.name());
        DCHECK(iter != context._attributes.end(), "Cannot find attribute %s", i.name().c_str());
        const Attribute& attribute = iter->second;
        divisors[attribute.divisor()].push_back(&i);
    }

    for(const auto& i : std::views::values(divisors))
        for(const ShaderPreprocessor::Declaration* j : i)
            locations.push_back(*j);

    return locations;
}

uint32_t RenderUtil::setLayoutDescriptor(const Vector<ShaderPreprocessor::Declaration>& declarations, const String& qualifierName, const uint32_t start, const int32_t set)
{
    uint32_t counter = start;
    for(const ShaderPreprocessor::Declaration& i : declarations)
        setLayoutQualifierBinding(i, qualifierName, getNextLayoutLocation(i, counter), set);
    return counter;
}

uint32_t RenderUtil::setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& declarations, const String& descriptor, const uint32_t start, const int32_t set)
{
    return setLayoutDescriptor(declarations.vars().values(), descriptor, start, set);
}

uint32_t RenderUtil::setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& ins, const ShaderPreprocessor::DeclarationList& outs, const String& qualifierName, const uint32_t start)
{
    uint32_t counter = start;
    DCHECK_WARN(ins.vars().size() == outs.vars().size(), "Output/Input mismatch, output and input have different numbers of items: [%s] vs [%s]",
                Strings::join(ins.vars().keys().data(), 0, ins.vars().keys().size()).c_str(), Strings::join(outs.vars().keys().data(), 0, outs.vars().keys().size()).c_str());
    for(const ShaderPreprocessor::Declaration& i : ins.vars().values()) {
        const uint32_t binding = getNextLayoutLocation(i, counter);
        setLayoutQualifierBinding(i, qualifierName, binding);
        const String outName = Strings::capitalizeFirst(i.name().startsWith("v_") ? i.name().substr(2) : i.name());
        const bool hasOutName = outs.vars().has(outName);
        DCHECK_WARN(hasOutName, "Output/Input mismatch, \"%s\" exists in input but not found in next stage of shader", outName.c_str());
        if(hasOutName)
            setLayoutQualifierBinding(outs.vars().at(outName), qualifierName, binding);
    }
    return counter;
}

}
