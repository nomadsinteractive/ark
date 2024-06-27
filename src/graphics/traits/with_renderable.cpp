#include "graphics/traits/with_renderable.h"

#include "core/util/documents.h"

#include "graphics/base/layer_context.h"

namespace ark {

WithRenderable::ManifestFactory::ManifestFactory(BeanFactory& factory, const document& manifest)
    : _renderable(factory.ensureBuilder<Renderable>(manifest, "Renderable")), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest)), _transform_node(Documents::getAttribute(manifest, "transform-node"))
{
}

WithRenderable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifests(factory.makeBuilderList<Renderable, ManifestFactory>(manifest, "renderable"))
{
}

sp<Wirable> WithRenderable::BUILDER::build(const Scope& args)
{
    std::vector<Manifest> manifests;
    for(const auto& [renderable, layerContext, transformNode] : _manifests)
        manifests.push_back({renderable->build(args), layerContext->build(args), transformNode});
    return sp<Wirable>::make<WithRenderable>(std::move(manifests));
}

}
