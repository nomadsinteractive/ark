#include "app/traits/with_text.h"

#include "graphics/base/boundaries.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"

#include "app/view/view.h"
#include "graphics/util/mat4_type.h"

namespace ark {

WithText::WithText(sp<Text> text, String transformNode)
    : _text(std::move(text)), _transform_node(std::move(transformNode))
{
}

TypeId WithText::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithText::onWire(const WiringContext& context)
{
    sp<Vec3> position = context.getComponent<Vec3>();
    if(sp<Boundaries> boundaries = context.getComponent<Boundaries>())
    {
        if(position)
            position = Vec3Type::add(position, boundaries->aabbMin());
        _text->setBoundaries(std::move(boundaries));
    }
    if(position)
        _text->setPosition(std::move(position));
    if(!_text->layoutParam())
        if(const sp<View> view = context.getComponent<View>())
            _text->setLayoutParam(view->layoutParam());
    sp<Mat4> matrix;
    if(const sp<Transform> transform = context.getComponent<Transform>())
        matrix = transform;
    if(_transform_node)
    {
        const sp<Model> model = context.getComponent<Model>();
        CHECK(model, "Text with transform node \"%s\" has no model defined", _transform_node.c_str());
        const sp<Node> node = model->findNode(_transform_node);
        CHECK(node, "Text with transform node \"%s\" model has no node defined", _transform_node.c_str());
        matrix = matrix ? Mat4Type::matmul(matrix, node->transform()) : sp<Mat4>::make<Mat4::Const>(node->transform());
    }
    if(matrix)
        _text->setTransform(std::move(matrix));
    _text->show(context.getComponent<Expendable>());
}

WithText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text(factory.ensureBuilder<Text>(manifest, constants::TEXT)), _transform_node(Documents::getAttribute(manifest, "transform-node"))
{
}

sp<Wirable> WithText::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithText>(_text->build(args), _transform_node);
}

}
