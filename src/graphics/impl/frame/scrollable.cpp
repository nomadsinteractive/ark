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
      _renderer_maker(std::move(rendererMaker)), _size(std::move(size))
{
}

const sp<Size>& Scrollable::size()
{
    return _size;
}

const std::vector<Box>& Scrollable::cull(uint64_t timestamp)
{
    if(_scroller->update(timestamp))
    {
        const V3 scroll = _scroller->val();

        _culled_objects.clear();

        float sx, ex;
        float sy, ey;
        const Rect viewport(scroll.x(), scroll.y(), scroll.x() + width(), scroll.y() + height());
        const Rect viewportCache(viewport.left() - width(), viewport.top() - height(), viewport.right() + width(), viewport.bottom() + height());
        Math::modBetween<float>(viewport.left(), viewport.right(), _params._renderer_width, sx, ex);
        Math::modBetween<float>(viewport.top(), viewport.bottom(), _params._renderer_height, sy, ey);
        for(float i = sy; i < ey; i += _params._renderer_height)
            for(float j = sx; j < ex; j += _params._renderer_width)
            {
                const std::vector<Box>& culled = _renderer_pool.cull(_renderer_maker, j, i, viewportCache);
                _culled_objects.insert(_culled_objects.end(), culled.begin(), culled.end());
            }
    }
    return _culled_objects;
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

int32_t Scrollable::width() const
{
    return static_cast<int32_t>(_size->widthAsFloat());
}

int32_t Scrollable::height() const
{
    return static_cast<int32_t>(_size->heightAsFloat());
}

Scrollable::RendererPool::RendererPool(float rendererWidth, float rendererHeight)
    : _renderer_width(rendererWidth), _renderer_height(rendererHeight)
{
}

const std::vector<Box>& Scrollable::RendererPool::cull(RendererMaker& rendererMaker, float x, float y, const Rect& viewport)
{
    const auto iter = _renderers.find(RendererKey(x, y));
    if(iter != _renderers.end())
        return iter->second;

    std::vector<Box> renderer = rendererMaker.make(x, y);
    recycleOutOfFrustum(rendererMaker, viewport);
    return _renderers[RendererKey(x, y)] = std::move(renderer);
}

void Scrollable::RendererPool::recycleOutOfFrustum(RendererMaker& rendererMaker, const Rect& viewport)
{
    for(auto iter = _renderers.begin(); iter != _renderers.end(); ++iter)
    {
        const RendererKey& key = iter->first;
        if(!viewport.intersect(Rect(key.first, key.second, key.first + _renderer_width, key.second + _renderer_height)))
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
      _size(factory.ensureBuilder<Size>(manifest, constants::SIZE)), _row_count(Documents::ensureAttribute<int32_t>(manifest, "rows")),
      _col_count(Documents::ensureAttribute<int32_t>(manifest, "cols")), _renderer_width(factory.ensureBuilder<Numeric>(manifest, "renderer-width")),
      _renderer_height(factory.ensureBuilder<Numeric>(manifest, "renderer-height"))
{
}

sp<Scrollable> Scrollable::BUILDER_SCROLLABLE::build(const Scope& args)
{
    const Params params(_row_count, _col_count, _row_count / 2, _col_count / 2, _renderer_width->build(args)->val(), _renderer_height->build(args)->val());
    return sp<Scrollable>::make(_scroller->build(args), _renderer_maker->build(args), _size->build(args), params);
}

}
