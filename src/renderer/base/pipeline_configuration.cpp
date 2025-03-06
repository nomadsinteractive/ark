#include "renderer/base/pipeline_configuration.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/shader.h"
#include "renderer/impl/snippet/snippet_draw_compute.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/inf/snippet.h"
#include "renderer/inf/snippet_factory.h"

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

sp<Snippet> createCoreSnippet(sp<Snippet> next)
{
    sp<Snippet> coreSnippet = Ark::instance().renderController()->renderEngine()->context()->snippetFactory()->createCoreSnippet();
    DASSERT(coreSnippet);
    if(next)
        return sp<Snippet>::make<SnippetComposite>(std::move(coreSnippet), std::move(next));
    return coreSnippet;
}

class ComputeSnippetWrapper final : public Snippet, public Wrapper<Snippet> {
public:
    ComputeSnippetWrapper()
        : Wrapper() {
    }

    sp<DrawDecorator> makeDrawDecorator(const RenderRequest& renderRequest) override
    {
        return _wrapped->makeDrawDecorator(renderRequest);
    }
};

}

PipelineConfiguration::PipelineConfiguration(sp<PipelineBuildingContext> buildingContext)
    : _building_context(std::move(buildingContext)), _pipeline_layout(_building_context->_pipeline_layout), _predefined_samplers(std::move(_building_context->_samplers)), _predefined_images(std::move(_building_context->_images)), _definitions(_building_context->toDefinitions())
{
}

void PipelineConfiguration::addSnippet(sp<Snippet> snippet)
{
    DASSERT(snippet);
    _snippet = _snippet ? sp<Snippet>::make<SnippetComposite>(_snippet, std::move(snippet)) : std::move(snippet);
}

void PipelineConfiguration::preCompile(GraphicsContext& graphicsContext)
{
    if(_building_context)
    {
        _snippet->preCompile(graphicsContext, _building_context, *this);

        for(const ShaderPreprocessor* preprocessor : _building_context->stages())
            _stages.push_back(preprocessor->preprocess());

        _building_context = nullptr;
    }
}

const sp<PipelineLayout>& PipelineConfiguration::pipelineLayout() const
{
    return _pipeline_layout;
}

Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> PipelineConfiguration::getPreprocessedStages(const RenderEngineContext& renderEngineContext) const
{
    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> shaders;

    for(const auto& [manifest, stage, source] : _stages)
        shaders[stage] = {manifest, stage, preprocess(renderEngineContext, _definitions, source)};

    return shaders;
}

void PipelineConfiguration::initialize(const Shader& shader)
{
    sp<ComputeSnippetWrapper> computeSnippetWrapper;
    if(const op<ShaderPreprocessor>& computeStage = _building_context->computingStage(); computeStage && !_building_context->renderStages().empty())
    {
        computeSnippetWrapper = sp<ComputeSnippetWrapper>::make();
        addSnippet(computeSnippetWrapper);
    }

    _snippet = createCoreSnippet(std::move(_snippet));
    _snippet->preInitialize(_building_context);
    _building_context->initialize(shader.camera());
    _pipeline_layout->initialize(_building_context);

    if(computeSnippetWrapper)
    {
        std::array<uint32_t, 3> numWorkGroupsArray = {1, 1, 1};
        const op<ShaderPreprocessor>& computeStage = _building_context->computingStage();
        if(const String numWorkGroupsAttr = Documents::getAttribute(computeStage->_manifest, "num-work-groups"))
        {
            const Vector<String> numWorkGroups = numWorkGroupsAttr.split(',');
            for(size_t i = 0; i < std::min(numWorkGroups.size(), numWorkGroupsArray.size()); ++i)
                numWorkGroupsArray[i] = Strings::eval<uint32_t>(numWorkGroups.at(i));
        }
        else
        {
            CHECK(computeStage->_compute_local_sizes, "Compute stage local size layout undefined");
            numWorkGroupsArray = computeStage->_compute_local_sizes.value();
        }
        computeSnippetWrapper->reset(sp<Snippet>::make<SnippetDrawCompute>(shader.layout(), numWorkGroupsArray, computeStage->_pre_shader_stage != Enum::SHADER_STAGE_BIT_NONE));
    }
}

Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> PipelineConfiguration::makeBindingSamplers() const
{
    const PipelineLayout& pipelineLayout = _pipeline_layout;
    CHECK_WARN(pipelineLayout._samplers.size() >= _predefined_samplers.size(), "Predefined samplers(%d) is more than samplers(%d) in PipelineLayout", _predefined_samplers.size(), pipelineLayout._samplers.size());

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> samplers;
    for(size_t i = 0; i < pipelineLayout._samplers.size(); ++i)
    {
        const String& name = pipelineLayout._samplers.keys().at(i);
        const auto iter = _predefined_samplers.find(name);
        samplers.emplace_back(iter != _predefined_samplers.end() ? iter->second : (i < _predefined_samplers.size() ? _predefined_samplers.values().at(i) : nullptr), pipelineLayout._samplers.values().at(i));
    }
    return samplers;
}

Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> PipelineConfiguration::makeBindingImages() const
{
    const PipelineLayout& pipelineLayout = _pipeline_layout;
    DASSERT(_predefined_images.size() == pipelineLayout._images.size());

    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> bindingImages;
    for(size_t i = 0; i < pipelineLayout._images.size(); ++i)
        bindingImages.emplace_back(_predefined_images.values().at(i), pipelineLayout._images.values().at(i));
    return bindingImages;
}

const sp<Snippet>& PipelineConfiguration::snippet() const
{
    return _snippet;
}

}
