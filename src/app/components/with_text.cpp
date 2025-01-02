#include "app/components/with_text.h"

#include "graphics/base/boundaries.h"
#include "graphics/components/with_transform.h"
#include "graphics/util/mat4_type.h"
#include "graphics/util/vec3_type.h"

#include "renderer/base/model.h"

#include "app/view/view.h"

namespace ark {

WithText::WithText(sp<Text> text, String nodeName, String viewName)
    : _text(std::move(text)), _node_name(std::move(nodeName)), _view_name(std::move(viewName))
{
}

TypeId WithText::onPoll(WiringContext& context)
{
    context.addComponent(_text);
    return constants::TYPE_ID_NONE;
}

void WithText::onWire(const WiringContext& context)
{
    _text->onWire(context);

    if(_view_name)
        _text->setBoundaries(context.ensureComponent<View>()->findView(_view_name)->makeBoundaries());

    if(_node_name)
    {
        const sp<Model> model = context.getComponent<Model>();
        CHECK(model, "Text with transform node \"%s\" has no model defined", _node_name.c_str());
        const sp<Node> node = model->findNode(_node_name);
        CHECK(node, "Text with transform node \"%s\" model has no node defined", _node_name.c_str());
        sp<Mat4> matrix = _text->transform();
        matrix = matrix ? Mat4Type::matmul(std::move(matrix), node->localMatrix()) : sp<Mat4>::make<Mat4::Const>(node->localMatrix());
        if(matrix)
            _text->setTransform(std::move(matrix));
    }
}

WithText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text(factory.ensureBuilder<Text>(manifest, constants::TEXT)), _node_name(Documents::getAttribute(manifest, "node-name")), _view_name(Documents::getAttribute(manifest, "view-name"))
{
}

sp<Wirable> WithText::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithText>(_text->build(args), _node_name, _view_name);
}

}
