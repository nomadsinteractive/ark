#include "renderer/impl/renderer/movieclip_renderer.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/inf/iterator.h"
#include "core/inf/variable.h"

namespace ark {

MovieclipRenderer::MovieclipRenderer(const sp<Movieclip>& movieclip, const sp<Numeric>& duration, float interval)
    : _movieclip(movieclip), _duration(duration), _interval(interval), _frame_id(0)
{
    DCHECK(_interval > 0, "Movieclip interval must be greater than 0");
}

void MovieclipRenderer::render(RenderRequest& renderRequest, float x, float y)
{
    uint32_t frameid = static_cast<uint32_t>(_duration->val() / _interval) + 1;
    if(frameid != _frame_id)
    {
        _frame_id = frameid;
        _current = _movieclip->next();
    }
    if(_current)
        _current->render(renderRequest, x, y);
    else
        _expired = true;
}

MovieclipRenderer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _movieclip(factory.ensureBuilder<Movieclip>(manifest, Constants::Attributes::MOVIECLIP)), _duration(factory.getBuilder<Numeric>(manifest, "t", false)),
      _interval(Documents::getAttribute<Clock::Interval>(manifest, Constants::Attributes::INTERVAL, 1000000 / 24).sec())
{
}

sp<Renderer> MovieclipRenderer::BUILDER::build(const sp<Scope>& args)
{
    return sp<MovieclipRenderer>::make(_movieclip->build(args), _duration ? _duration->build(args) : Ark::instance().clock()->duration(), _interval);
}

}
