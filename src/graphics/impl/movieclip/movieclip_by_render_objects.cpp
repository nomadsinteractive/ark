#include "graphics/impl/movieclip/movieclip_by_render_objects.h"

#include "core/base/bean_factory.h"

#include "graphics/base/render_object.h"
#include "graphics/impl/renderer/renderer_by_render_object.h"

namespace ark {

MovieclipByRenderObjects::MovieclipByRenderObjects(const sp<Layer>& layer, const sp<Transform>& transform, const std::list<sp<Builder<RenderObject>>>& renderObjects, const sp<Scope>& args)
{
    for(const sp<Builder<RenderObject>>& i : renderObjects)
    {
        const sp<RenderObject> renderObject = i->build(args);
        if(transform)
            renderObject->setTransform(transform);
        _movieclips.push_back(sp<RendererByRenderObject>::make(renderObject, layer));
    }
    _iterator = _movieclips.begin();
}

bool MovieclipByRenderObjects::hasNext()
{
    return !_movieclips.empty() && _iterator != _movieclips.end();
}

sp<Renderer> MovieclipByRenderObjects::next()
{
    if(_iterator == _movieclips.end())
    {
        _movieclips.clear();
        return nullptr;
    }

    const sp<Renderer> renderer = *_iterator;
    ++_iterator;
    return renderer;
}

MovieclipByRenderObjects::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _layer(factory.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER)), _transform(factory.getBuilder<Transform>(manifest, Constants::Attributes::TRANSFORM))
{
    for(const document& i : manifest->children())
        _render_objects.push_back(factory.ensureBuilder<RenderObject>(i));
}

sp<Movieclip> MovieclipByRenderObjects::BUILDER::build(const sp<Scope>& args)
{
    const sp<Layer> layer = _layer->build(args);
    return sp<MovieclipByRenderObjects>::make(layer, _transform->build(args), _render_objects, args);
}

}
