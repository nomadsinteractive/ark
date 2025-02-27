#include "renderer/base/pipeline_layout.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/shader.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/impl/snippet/snippet_draw_compute.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet.h"

namespace ark {

namespace {

String preprocess(const RenderEngineContext& renderEngineContext, const Map<String, String>& definitions, const String& source)
{
    DCHECK(renderEngineContext.version() > 0, "Unintialized RenderEngineContext");

    static std::regex var_pattern(R"(\$\{([\w.]+)\})");
    const Map<String, String>& engineDefinitions = renderEngineContext.definitions();

    return source.replace(var_pattern, [&engineDefinitions, &definitions] (Array<String>& matches)->String {
        const String& varName = matches.at(1);
        auto iter = engineDefinitions.find(varName);
        if(iter != engineDefinitions.end())
            return iter->second;
        iter = definitions.find(varName);
        CHECK(iter != definitions.end(), "Undefinition \"%s\"", varName.c_str());
        return iter->second;
    });
}

}

PipelineLayout::PipelineLayout(sp<PipelineBuildingContext> buildingContext)
    : _building_context(std::move(buildingContext)), _shader_layout(_building_context->_shader_layout), _predefined_samplers(std::move(_building_context->_samplers)), _predefined_images(std::move(_building_context->_images)), _color_attachment_count(0),
      _definitions(_building_context->toDefinitions())
{
}

void PipelineLayout::addSnippet(sp<Snippet> snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::make<SnippetLinkedChain>(_snippet, std::move(snippet)) : std::move(snippet);
}

void PipelineLayout::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        for(const ShaderPreprocessor* preprocessor : _building_context->stages())
            _preprocessed_stages.push_back(preprocessor->preprocess());

        _building_context = nullptr;
    }
}

const sp<ShaderLayout>& PipelineLayout::shaderLayout() const
{
    return _shader_layout;
}

Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> PipelineLayout::getPreprocessedStages(const RenderEngineContext& renderEngineContext) const
{
    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> shaders;

    for(const auto& [manifest, stage, source] : _preprocessed_stages)
        shaders[stage] = {manifest, stage, preprocess(renderEngineContext, _definitions, source)};

    return shaders;
}

size_t PipelineLayout::colorAttachmentCount() const
{
    return _color_attachment_count;
}

void PipelineLayout::initialize(const Shader& shader)
{
    if(const op<ShaderPreprocessor>& computeStage = _building_context->computingStage(); computeStage && !_building_context->renderStages().empty())
    {
        std::array<uint32_t, 3> numWorkGroupsArray = {1, 1, 1};
        const Vector<String> numWorkGroups = Documents::getAttribute(computeStage->_manifest, "num-work-groups", "64").split(',');
        for(size_t i = 0; i < std::min(numWorkGroups.size(), numWorkGroupsArray.size()); ++i)
            numWorkGroupsArray[i] = Strings::eval<uint32_t>(numWorkGroups.at(i));
        addSnippet(sp<Snippet>::make<SnippetDrawCompute>(shader.input(), numWorkGroupsArray, true));
    }
    _snippet = sp<Snippet>::make<SnippetDelegate>(_snippet);
    _snippet->preInitialize(_building_context);
    _building_context->initialize(*this);

    if(const ShaderPreprocessor* fragment = _building_context->tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        _color_attachment_count = fragment->_main_block->outArgumentCount() + (fragment->_main_block->hasReturnValue() ? 1 : 0);

    _shader_layout->initialize(_building_context);
}

Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> PipelineLayout::makeBindingSamplers() const
{
    const ShaderLayout& shaderLayout = _shader_layout;
    CHECK_WARN(shaderLayout._samplers.size() >= _predefined_samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", _predefined_samplers.size(), shaderLayout._samplers.size());

    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> samplers;
    for(size_t i = 0; i < shaderLayout._samplers.size(); ++i)
    {
        const String& name = shaderLayout._samplers.keys().at(i);
        const auto iter = _predefined_samplers.find(name);
        samplers.emplace_back(iter != _predefined_samplers.end() ? iter->second : (i < _predefined_samplers.size() ? _predefined_samplers.values().at(i) : nullptr), shaderLayout._samplers.values().at(i));
    }
    return samplers;
}

Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> PipelineLayout::makeBindingImages() const
{
    const ShaderLayout& shaderLayout = _shader_layout;
    DASSERT(_predefined_images.size() == shaderLayout._images.size());

    Vector<std::pair<sp<Texture>, ShaderLayout::DescriptorSet>> bindingImages;
    for(size_t i = 0; i < shaderLayout._images.size(); ++i)
        bindingImages.emplace_back(_predefined_images.values().at(i), shaderLayout._images.values().at(i));
    return bindingImages;
}

const sp<Snippet>& PipelineLayout::snippet() const
{
    return _snippet;
}

}
