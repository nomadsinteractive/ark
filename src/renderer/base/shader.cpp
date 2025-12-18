#include "renderer/base/shader.h"

#include <ranges>

#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/string_convert.h"
#include "core/util/documents.h"

#include "graphics/base/camera.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/snippet/snippet_composite.h"

namespace ark {

namespace {

std::pair<Optional<const Shader::StageManifest&>, size_t> findStageManifest(const enums::ShaderStageBit type, const Vector<Shader::StageManifest>& stages)
{
    for(size_t i = 0; i < stages.size(); ++ i)
        if(type == stages.at(i)._type)
            return {stages.at(i), i};
    return {{}, 0};
}

template<typename T, typename U> auto findInKeywordPairs(const T& kws, U key) -> decltype(std::declval<T>().front().second)
{
    for(const auto& [k, v] : kws)
        if(k == key)
            return v;
    return {};
}

std::pair<String, document> findStageSourceAndManifest(const enums::ShaderStageBit shaderStage, const Optional<const Shader::StageManifest&>& stageManifest, const Vector<Shader::StageManifest>& defaultStages, const String& defaultShaderSource, const Scope& args)
{
    if(stageManifest)
        return {stageManifest->_source->build(args), stageManifest->_manifest};

    if(const auto [stageManifestOpt, _] = findStageManifest(shaderStage, defaultStages); stageManifestOpt)
        return {stageManifestOpt->_source->build(args), stageManifestOpt->_manifest};

    const Global<StringTable> globalStringTable;
    document manifest = document::make("stage", "", Vector{attribute::make("src", defaultShaderSource)});
    return {std::move(globalStringTable->getString("shaders", defaultShaderSource, true).value()), std::move(manifest)};
}

}

Shader::StageManifest::StageManifest(const enums::ShaderStageBit type, sp<IBuilder<String>> source)
    : _type(type), _source(std::move(source)), _manifest(Global<Constants>()->DOCUMENT_NONE)
{
}

Shader::StageManifest::StageManifest(BeanFactory& factory, const document& manifest)
    : _type(Documents::ensureAttribute<enums::ShaderStageBit>(manifest, constants::TYPE)), _source(factory.ensureIBuilder<String>(manifest, constants::SRC)), _manifest(manifest)
{
}

Shader::Shader(sp<PipelineDescriptor> pipelineDescriptor)
    : _pipeline_desciptor(std::move(pipelineDescriptor))
{
}

sp<Builder<Shader>> Shader::makeBuilder(BeanFactory& factory, const document& manifest, const String& defVertex, const String& defFragment)
{
    const Global<StringTable> stringTable;
    Vector<StageManifest> defaultManifests;
    if(defVertex)
        defaultManifests.emplace_back(enums::SHADER_STAGE_BIT_VERTEX, sp<IBuilder<String>>::make<IBuilder<String>::Prebuilt>(std::move(stringTable->getString(defVertex, true).value())));
    if(defFragment)
        defaultManifests.emplace_back(enums::SHADER_STAGE_BIT_FRAGMENT, sp<IBuilder<String>>::make<IBuilder<String>::Prebuilt>(std::move(stringTable->getString(defFragment, true).value())));

    if(const sp<Builder<Shader>> shader = factory.getBuilder<Shader>(manifest, constants::SHADER))
    {
        sp<BUILDER> shaderBuilder = shader.cast<BUILDER>();
        shaderBuilder->_default_stages = std::move(defaultManifests);
        return shaderBuilder;
    }

    return sp<Builder<Shader>>::make<BUILDER>(factory, manifest, std::move(defaultManifests));
}

sp<RenderBufferSnapshot> Shader::takeBufferSnapshot(const RenderRequest& renderRequest, const bool isComputeStage) const
{
    return _pipeline_desciptor->layout()->takeBufferSnapshot(renderRequest, isComputeStage);
}

const Camera& Shader::camera() const
{
    return _pipeline_desciptor->camera();
}

const sp<PipelineLayout>& Shader::layout() const
{
    return _pipeline_desciptor->layout();
}

void Shader::setCamera(const Camera& camera)
{
    _pipeline_desciptor->camera().assign(camera);
}

const sp<PipelineDescriptor>& Shader::pipelineDesciptor() const
{
    return _pipeline_desciptor;
}

sp<PipelineBindings> Shader::makeBindings(Buffer vertexBuffer, enums::DrawMode drawMode, enums::DrawProcedure drawProcedure, Vector<std::pair<uint32_t, Buffer>> instanceBuffers) const
{
    for(const auto& divisor : _pipeline_desciptor->layout()->streamLayouts() | std::views::keys)
        if(divisor != 0 && !findInKeywordPairs(instanceBuffers, divisor))
            instanceBuffers.emplace_back(divisor, Ark::instance().renderController()->makeVertexBuffer(Buffer::USAGE_BIT_DYNAMIC));

    return sp<PipelineBindings>::make(drawMode, drawProcedure, std::move(vertexBuffer), _pipeline_desciptor, std::move(instanceBuffers));
}

Shader::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, Vector<StageManifest> defaultStages)
    : _factory(factory), _manifest(manifest), _stages(factory.makeBuilderListObject<StageManifest>(manifest, "stage")), _default_stages(std::move(defaultStages)),
      _snippets(factory.makeBuilderList<Snippet>(manifest, "snippet")), _camera(factory.getBuilder<Camera>(manifest, constants::CAMERA)),
      _configuration(factory, manifest)
{
}

sp<Shader> Shader::BUILDER::build(const Scope& args)
{
    return build(args, _default_stages, _camera.build(args));
}

sp<Shader> Shader::BUILDER::build(const Scope& args, const Vector<StageManifest>& defaultStages, sp<Camera> defaultCamera)
{
    sp<Snippet> snippet;
    for(const sp<Builder<Snippet>>& i : _snippets)
        snippet = SnippetComposite::compose(std::move(snippet), i->build(args));

    PipelineBuildingContext buildingContext = makePipelineBuildingContext(args, defaultStages);
    const sp<Camera> camera = defaultCamera ? std::move(defaultCamera) : _camera.build(args);
    PipelineDescriptor::Configuration configuration = _configuration.build(args);
    configuration._snippet = std::move(snippet);
    return sp<Shader>::make(sp<PipelineDescriptor>::make(camera ? *camera : Camera::createDefaultCamera(), buildingContext, std::move(configuration)));
}

PipelineBuildingContext Shader::BUILDER::makePipelineBuildingContext(const Scope& args, const Vector<StageManifest>& defaultStages) const
{
    PipelineBuildingContext context(_factory, args);
    const auto [vertexOpt, vertexIndex] = findStageManifest(enums::SHADER_STAGE_BIT_VERTEX, _stages);
    const auto [fragmentOpt, fragmentIndex] = findStageManifest(enums::SHADER_STAGE_BIT_FRAGMENT, _stages);
    const auto [computeOpt, computeIndex] = findStageManifest(enums::SHADER_STAGE_BIT_COMPUTE, _stages);
    if(vertexOpt || fragmentOpt || !computeOpt)
    {
        auto [vertexSource, vertexManifest] = findStageSourceAndManifest(enums::SHADER_STAGE_BIT_VERTEX, vertexOpt, defaultStages, "default.vert", args);
        auto [fragmentSource, fragmentManifest] = findStageSourceAndManifest(enums::SHADER_STAGE_BIT_FRAGMENT, fragmentOpt, defaultStages, "texture.frag", args);
        context.addStage(std::move(vertexSource), std::move(vertexManifest), enums::SHADER_STAGE_BIT_VERTEX, enums::SHADER_STAGE_BIT_NONE);
        context.addStage(std::move(fragmentSource), std::move(fragmentManifest), enums::SHADER_STAGE_BIT_FRAGMENT, enums::SHADER_STAGE_BIT_VERTEX);
    }
    else
        CHECK(computeOpt, "Shader must have at least one stage defined");

    if(computeOpt)
    {
        const bool computeAfterGraphics = (vertexOpt && vertexIndex < computeIndex) || (fragmentOpt && fragmentIndex < computeIndex);
        context.addStage(computeOpt->_source->build(args), computeOpt->_manifest, enums::SHADER_STAGE_BIT_COMPUTE, computeAfterGraphics ? enums::SHADER_STAGE_BIT_FRAGMENT : enums::SHADER_STAGE_BIT_NONE);
    }

    context.loadManifest(_manifest);
    return context;
}

template<> ARK_API enums::ShaderStageBit StringConvert::eval<enums::ShaderStageBit>(const String& expr)
{
    constexpr enums::LookupTable<enums::ShaderStageBit, 3> table = {{
        {"vertex", enums::SHADER_STAGE_BIT_VERTEX},
        {"fragment", enums::SHADER_STAGE_BIT_FRAGMENT},
        {"compute", enums::SHADER_STAGE_BIT_COMPUTE}
    }};
    return enums::lookup(table, expr);
}

}
