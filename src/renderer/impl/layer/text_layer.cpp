#include "renderer/impl/layer/text_layer.h"

#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_model/render_model_text.h"
#include "renderer/impl/snippet/snippet_ucolor.h"

namespace ark {

namespace  {

class ShaderBuilder : public Builder<Shader> {
public:
    ShaderBuilder(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
        : _resource_loader_context(resourceLoaderContext), _shader(factory.getBuilder<Shader>(manifest, Constants::Attributes::SHADER)),
          _color(factory.getBuilder<Vec4>(manifest, Constants::Attributes::TEXT_COLOR)) {
    }

    virtual sp<Shader> build(const Scope& args) override {
        if(_shader)
            return _shader->build(args);

        const sp<Vec4> color = _color ? _color->build(args) : sp<Color>::make(Color::WHITE).cast<Vec4>();
        return Shader::fromStringTable("shaders/default.vert", "shaders/alpha.frag", sp<SnippetUColor>::make(color), _resource_loader_context);
    }

private:
    sp<ResourceLoaderContext> _resource_loader_context;
    sp<Builder<Shader>> _shader;
    sp<Builder<Vec4>> _color;
};

}

TextLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<RenderModelText::BUILDER>::make(factory, manifest, resourceLoaderContext), sp<ShaderBuilder>::make(factory, manifest, resourceLoaderContext))
{
}

sp<RenderLayer> TextLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
