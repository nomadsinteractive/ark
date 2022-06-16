#include "app/impl/renderer/fps_counter.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/base/clock.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"

#include "renderer/base/characters.h"

#include "app/base/application_context.h"

namespace ark {

FPSCounter::FPSCounter(const sp<Clock>& clock, float refreshInterval, const sp<Characters>& characters, const String& message)
    : _characters(characters), _message(message), _frame_rendered(0),
      _duration(clock->duration()), _refresh_interval(refreshInterval), _last_refresh(0)
{
}

const sp<Size>& FPSCounter::size()
{
    return _characters->size();
}

void FPSCounter::render(RenderRequest& renderequest, const V3& position)
{
    ++ _frame_rendered;
    float duration = _duration->val();
    float elapsed = duration - _last_refresh;
    if(elapsed > _refresh_interval)
    {
        if(_last_refresh != 0)
            updateFPS(_frame_rendered / elapsed);
        _last_refresh = duration;
        _frame_rendered = 0;
    }
    _characters->renderRequest(renderequest, position);
}

void FPSCounter::updateFPS(float fps)
{
    const String text = Strings::sprintf(_message.c_str(), fps);
    LOGD(text.c_str());
    _characters->setText(Strings::fromUTF8(text));
}

FPSCounter::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _characters(factory.ensureConcreteClassBuilder<Characters>(manifest, "characters")),
      _message(Documents::getAttribute(manifest, "message", "FPS %.1f")),
      _interval(Documents::getAttribute<Clock::Interval>(manifest, Constants::Attributes::INTERVAL, 2000000).sec())
{
}

sp<Renderer> FPSCounter::BUILDER::build(const Scope& args)
{
    return sp<FPSCounter>::make(Ark::instance().clock(), _interval, _characters->build(args), _message);
}

}
