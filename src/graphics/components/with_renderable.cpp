#include "graphics/components/with_renderable.h"

#include "core/components/discarded.h"
#include "core/components/with_id.h"
#include "core/util/documents.h"

#include "graphics/base/layer_context.h"
#include "graphics/components/render_object.h"
#include "graphics/impl/renderable/renderable_with_transform.h"
#include "graphics/util/mat4_type.h"

#include "renderer/base/model.h"
#include "renderer/base/shader.h"

namespace ark {

WithRenderable::WithRenderable(Vector<Manifest> manifests)
    : _manifests(std::move(manifests))
{
}

void WithRenderable::onWire(const WiringContext& context, const Box& self)
{
    const sp<Boolean> discarded = context.getComponent<Discarded>();
    const sp<Boolean> visible = context.getComponent<Visibility>();
    const sp<Model> model = context.getComponent<Model>();
    const sp<Mat4> transform = context.getComponent<Transform>();
    for(const auto& [layer, renderable, renderObject, transformNode] : _manifests)
    {
        if(renderObject)
            renderObject->onWire(context, self);
        else
        {
            sp<Renderable> r = renderObject ? renderObject.cast<Renderable>() : renderable;
            const sp<Node> node = model && transformNode ? model->findNode(transformNode) : nullptr;
            CHECK(!transformNode || node, "Transform node \"%s\" doesn't exist", transformNode.c_str());
            if(transform)
                r = sp<Renderable>::make<RenderableWithTransform>(std::move(r), node ? Mat4Type::matmul(transform, node->localMatrix()) : transform);
            else if(node)
                r = sp<Renderable>::make<RenderableWithTransform>(std::move(r), sp<Mat4>::make<Mat4::Const>(node->localMatrix()));
            layer->pushBack(std::move(r), discarded);
        }
    }
}

WithRenderable::ManifestFactory::ManifestFactory(BeanFactory& factory, const document& manifest)
    : _layer(factory.ensureBuilder<Layer>(manifest, constants::LAYER)), _transform_node(Documents::getAttribute(manifest, "transform-node"))
{
    if(manifest->name() == constants::RENDER_OBJECT)
        _render_object = factory.ensureBuilder<RenderObject>(manifest);
    else
    {
        CHECK_WARN(manifest->name() == "renerable", "Name \"Renderable\" expected, found \"%s\"", manifest->name().c_str());
        _renderable = factory.ensureBuilder<Renderable>(manifest, "renderable");
    }
}

WithRenderable::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifests(factory.makeBuilderListObject<ManifestFactory>(manifest, constants::RENDER_OBJECT))
{
}

sp<Wirable> WithRenderable::BUILDER::build(const Scope& args)
{
    Vector<Manifest> manifests;
    manifests.reserve(_manifests.size());
    for(const auto& [layer, renderable, renderObject, transformNode] : _manifests)
        manifests.push_back({layer->build(args), renderable ? renderable->build(args) : nullptr, renderObject ? renderObject->build(args) : nullptr, transformNode});
    return sp<Wirable>::make<WithRenderable>(std::move(manifests));
}

}
