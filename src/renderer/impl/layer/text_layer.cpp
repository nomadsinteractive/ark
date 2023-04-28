#include "renderer/impl/layer/text_layer.h"

#include "core/base/string_table.h"

#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/model_loader/model_loader_text.h"
#include "renderer/impl/snippet/snippet_ucolor.h"

namespace ark {

namespace  {

class SnippetBuilder : public Builder<Snippet> {
public:
    SnippetBuilder(BeanFactory& factory, const document& manifest)
        : _text_color(factory.getBuilder<Vec4>(manifest, Constants::Attributes::TEXT_COLOR)) {
    }

    virtual sp<Snippet> build(const Scope& args) override {
        sp<Vec4> textColor = _text_color ? _text_color->build(args) : sp<Vec4>::make<Color>(Color::WHITE);
        return sp<SnippetUColor>::make(textColor);
    }

private:
    sp<Builder<Vec4>> _text_color;

};

sp<Builder<Shader>> makeShaderBuilder(BeanFactory& factory, const document& manifest, const ResourceLoaderContext& resourceLoaderContext) {
    const Global<StringTable> stringTable;
    sp<String> vertexStr = stringTable->getString("shaders/default.vert", true);
    sp<String> fragmentStr = stringTable->getString("shaders/alpha.frag", true);

    Shader::StageManifest stageManifest = {
        {PipelineInput::SHADER_STAGE_VERTEX, sp<Builder<String>::Prebuilt>::make(std::move(vertexStr))},
        {PipelineInput::SHADER_STAGE_FRAGMENT, sp<Builder<String>::Prebuilt>::make(std::move(fragmentStr))}
    };
    Shader::SnippetManifest snippetManifest = {sp<SnippetBuilder>::make(factory, manifest)};

    return sp<Shader::BUILDER_IMPL>::make(factory, manifest, resourceLoaderContext, nullptr, std::move(stageManifest), std::move(snippetManifest));
}

}

TextLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<ModelLoaderText::BUILDER>::make(factory, manifest, resourceLoaderContext), makeShaderBuilder(factory, manifest, resourceLoaderContext))
{
}

sp<RenderLayer> TextLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
