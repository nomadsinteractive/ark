#include "graphics/traits/with_renderable.h"

#include "core/util/documents.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_object.h"
#include "graphics/impl/renderable/renderable_with_transform.h"
#include "graphics/util/mat4_type.h"

#include "renderer/base/model.h"

namespace ark {

namespace {

sp<Builder<Renderable>> ensureRenderableBuilder(BeanFactory& factory, const document& manifest)
{
    if(manifest->name() == constants::RENDER_OBJECT)
        return sp<Builder<Renderable>>::make<Builder<Renderable>::Wrapper<Builder<RenderObject>>>(factory.ensureBuilder<RenderObject>(manifest));
    return factory.ensureBuilder<Renderable>(manifest, "renderable");
}

}

WithRenderable::WithRenderable(std::vector<Manifest> manifests)
    : _manifests(std::move(manifests))
{
}

TypeId WithRenderable::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithRenderable::onWire(const WiringContext& context)
{
    const sp<Boolean> discarded = context.getComponent<Expendable>();
    const sp<Boolean> visible = context.getComponent<Visibility>();
    const sp<Model> model = context.getComponent<Model>();
    const sp<Transform> transform = context.getComponent<Transform>();
    for(const auto& [renderable, layerContext, transformNode] : _manifests)
    {
        sp<Renderable> r = renderable;
        const sp<Node> node = model && transformNode ? model->findNode(transformNode) : nullptr;
        if(transform)
            r = sp<Renderable>::make<RenderableWithTransform>(std::move(r), node ? Mat4Type::matmul(transform, node->transform()) : transform.cast<Mat4>());
        else if(node)
            r = sp<Renderable>::make<RenderableWithTransform>(std::move(r), sp<Mat4>::make<Mat4::Const>(node->transform()));
        layerContext->add(std::move(r), visible, discarded);
    }
}

WithRenderable::ManifestFactory::ManifestFactory(BeanFactory& factory, const document& manifest)
    : _renderable(ensureRenderableBuilder(factory, manifest)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest)), _transform_node(Documents::getAttribute(manifest, "transform-node"))
{
}

WithRenderable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifests(factory.makeBuilderListObject<ManifestFactory>(manifest, "render_object"))
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
