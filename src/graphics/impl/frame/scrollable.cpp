#include "graphics/impl/frame/scrollable.h"

#include "core/base/bean_factory.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/size.h"
#include "graphics/base/v3.h"
#include "graphics/base/rect.h"

namespace ark {

Scrollable::Scrollable(sp<Vec3> scroller, sp<RendererMaker> rendererMaker, sp<Size> size, const Scrollable::Params& params)
    : _params(params), _renderer_pool(_params._renderer_width, _params._renderer_height), _scroller(std::move(scroller)),
      _renderer_maker(std::move(rendererMaker)), _size(std::move(size)), _scroll_x(0), _scroll_y(0)
{
}

void Scrollable::render(RenderRequest& renderRequest, const V3& position)
{
    update();

    int32_t sx, ex;
    int32_t sy, ey;
    const RectI viewport(_scroll_x, _scroll_y, _scroll_x + width(), _scroll_y + height());
    const RectI viewportCache(viewport.left() - width(), viewport.top() - height(), viewport.right() + width(), viewport.bottom() + height());
    Math::modBetween<int32_t>(viewport.left(), viewport.right(), static_cast<int32_t>(_params._renderer_width), sx, ex);
    Math::modBetween<int32_t>(viewport.top(), viewport.bottom(), static_cast<int32_t>(_params._renderer_height), sy, ey);
    for(int32_t i = sy; i < ey; i += _params._renderer_height)
        for(int32_t j = sx; j < ex; j += _params._renderer_width)
        {
//            for(const sp<Renderer>& k : _renderer_pool.cull(_renderer_maker, j, i, viewportCache))
//                k->render(renderRequest, position);
        }
}

const sp<Size>& Scrollable::size()
{
    return _size;
}

void Scrollable::cull()
{
    update();

    int32_t sx, ex;
    int32_t sy, ey;
    const RectI viewport(_scroll_x, _scroll_y, _scroll_x + width(), _scroll_y + height());
    const RectI viewportCache(viewport.left() - width(), viewport.top() - height(), viewport.right() + width(), viewport.bottom() + height());
    Math::modBetween<int32_t>(viewport.left(), viewport.right(), static_cast<int32_t>(_params._renderer_width), sx, ex);
    Math::modBetween<int32_t>(viewport.top(), viewport.bottom(), static_cast<int32_t>(_params._renderer_height), sy, ey);
    for(int32_t i = sy; i < ey; i += _params._renderer_height)
        for(int32_t j = sx; j < ex; j += _params._renderer_width)
            _renderer_pool.cull(_renderer_maker, j, i, viewportCache);
}

const sp<Vec3>& Scrollable::scroller() const
{
    return _scroller;
}

const sp<RendererMaker>& Scrollable::rendererMaker() const
{
    return _renderer_maker;
}

void Scrollable::setRendererMaker(const sp<RendererMaker>& rendererMaker)
{
    _renderer_maker = rendererMaker;
}

void Scrollable::update()
{
    const V3 scroll = _scroller->val();
    int32_t scrollX = static_cast<int32_t>(scroll.x());
    int32_t scrollY = static_cast<int32_t>(scroll.y());
    if(scrollX != _scroll_x || scrollY != _scroll_y)
    {
        _scroll_x = scrollX;
        _scroll_y = scrollY;
    }
}

int32_t Scrollable::width() const
{
    return static_cast<int32_t>(_size->width());
}

int32_t Scrollable::height() const
{
    return static_cast<int32_t>(_size->height());
}

Scrollable::RendererPool::RendererPool(int32_t rendererWidth, int32_t rendererHeight)
    : _renderer_width(rendererWidth), _renderer_height(rendererHeight)
{
}

const std::vector<Box>& Scrollable::RendererPool::cull(RendererMaker& rendererMaker, int32_t x, int32_t y, const RectI& viewport)
{
    const auto iter = _renderers.find(RendererKey(x, y));
    if(iter != _renderers.end())
        return iter->second;

    std::vector<Box> renderer = rendererMaker.make(x, y);
    recycleOutOfFrustum(rendererMaker, viewport);
    return _renderers[RendererKey(x, y)] = std::move(renderer);
}

void Scrollable::RendererPool::recycleOutOfFrustum(RendererMaker& rendererMaker, const RectI& viewport)
{
    for(auto iter = _renderers.begin(); iter != _renderers.end(); ++iter)
    {
        const RendererKey& key = iter->first;
        if(!viewport.intersect(RectI(key.first, key.second, key.first + _renderer_width, key.second + _renderer_height)))
        {
            for(const Box& i : iter->second)
                rendererMaker.recycle(i);
            if((iter = _renderers.erase(iter)) == _renderers.end())
                break;
        }
    }
}

Scrollable::Params::Params(int32_t rowCount, int32_t colCount, int32_t rowIndex, int32_t colIndex, int32_t rendererWidth, int32_t rendererHeight)
    : _row_count(rowCount), _col_count(colCount), _row_index(rowIndex), _col_index(colIndex), _renderer_width(rendererWidth), _renderer_height(rendererHeight)
{
}

Scrollable::BUILDER_SCROLLABLE::BUILDER_SCROLLABLE(BeanFactory& factory, const document& manifest)
    : _scroller(factory.getBuilder<Vec3>(manifest, "scroller")), _renderer_maker(factory.ensureBuilder<RendererMaker>(manifest, "renderer-maker")),
      _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)), _row_count(Documents::ensureAttribute<int32_t>(manifest, "rows")),
      _col_count(Documents::ensureAttribute<int32_t>(manifest, "cols")), _renderer_width(factory.ensureBuilder<Numeric>(manifest, "renderer-width")),
      _renderer_height(factory.ensureBuilder<Numeric>(manifest, "renderer-height"))
{
}

sp<Scrollable> Scrollable::BUILDER_SCROLLABLE::build(const Scope& args)
{
    const Params params(_row_count, _col_count, _row_count / 2, _col_count / 2, static_cast<int32_t>(_renderer_width->build(args)->val()),
                        static_cast<int32_t>(_renderer_height->build(args)->val()));
    return sp<Scrollable>::make(_scroller->build(args), _renderer_maker->build(args), _size->build(args), params);
}

Scrollable::BUILDER_RENDERER::BUILDER_RENDERER(BeanFactory& factory, const document& manifest)
    : _impl(factory, manifest)
{
}

sp<Renderer> Scrollable::BUILDER_RENDERER::build(const Scope& args)
{
    return _impl.build(args);
}

}
