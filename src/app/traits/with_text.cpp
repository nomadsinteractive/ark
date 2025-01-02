#include "app/traits/with_text.h"

#include "graphics/base/boundaries.h"
#include "graphics/traits/with_transform.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"

#include "app/view/view.h"

namespace ark {

WithText::WithText(sp<Text> text, String nodeName)
    : _text(std::move(text)), _node_name(std::move(nodeName))
{
}

TypeId WithText::onPoll(WiringContext& context)
{
    context.addComponent(_text);
    return constants::TYPE_ID_NONE;
}

void WithText::onWire(const WiringContext& context)
{
    if(sp<Boundaries> boundaries = context.getComponent<Boundaries>())
        _text->setBoundaries(std::move(boundaries));

    if(!_text->layoutParam())
        if(const sp<View> view = context.getComponent<View>())
            _text->setLayoutParam(view->layoutParam());

    sp<Mat4> matrix;
    if(const sp<WithTransform> transform = context.getComponent<WithTransform>())
        matrix = transform->transform();
    if(_node_name)
    {
        const sp<Model> model = context.getComponent<Model>();
        CHECK(model, "Text with transform node \"%s\" has no model defined", _node_name.c_str());
        const sp<Node> node = model->findNode(_node_name);
        CHECK(node, "Text with transform node \"%s\" model has no node defined", _node_name.c_str());
        matrix = matrix ? Mat4Type::matmul(std::move(matrix), node->localMatrix()) : sp<Mat4>::make<Mat4::Const>(node->localMatrix());
    }
    if(matrix)
        _text->setTransform(std::move(matrix));
    _text->show(context.getComponent<Discarded>());
}

WithText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text(factory.ensureBuilder<Text>(manifest, constants::TEXT)), _node_name(Documents::getAttribute(manifest, "node-name"))
{
}

sp<Wirable> WithText::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithText>(_text->build(args), _node_name);
}

}
