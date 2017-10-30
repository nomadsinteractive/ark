#include "app/impl/renderer/fps_counter.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/base/clock.h"
#include "core/util/documents.h"
#include "core/util/bean_utils.h"
#include "core/util/log.h"

#include "graphics/base/layer_context.h"

#include "renderer/impl/layer/image_layer.h"
#include "renderer/impl/layer/alphabet_layer.h"

#include "app/base/application_context.h"

namespace ark {

FPSCounter::FPSCounter(const sp<Clock>& clock, float refreshInterval, const sp<ImageLayer>& imageLayer, const String& message)
    : _image_layer(imageLayer), _message(message), _frame_rendered(0),
      _duration(clock->duration()), _refresh_interval(refreshInterval), _last_refresh(0)
{
}

const sp<Size>& FPSCounter::size()
{
    return Null::toSafe<Size>(_characters._size);
}

void FPSCounter::render(RenderCommandPipeline& /*pipeline*/, float x, float y)
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
    if(_characters._render_objects)
        for(const sp<RenderObject>& i : _characters._render_objects->items())
            _image_layer->renderContext()->draw(x, y, i);
}

void FPSCounter::updateFPS(float fps)
{
    const String text = Strings::sprintf(_message.c_str(), fps);
    LOGD(text.c_str());
    _characters = Alphabets::create(_image_layer->atlas(), Strings::fromUTF8(text));
}

FPSCounter::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _layer(parent.ensureBuilder<Layer>(doc, Constants::Attributes::LAYER)),
      _message(Documents::getAttribute(doc, "message", "FPS %.1f")),
      _interval(Documents::getAttribute<Clock::Interval>(doc, Constants::Attributes::INTERVAL, 2000000).sec())
{
}

sp<Renderer> FPSCounter::BUILDER::build(const sp<Scope>& args)
{
    const sp<Layer> layer = _layer->build(args);
    DCHECK(layer.is<AlphabetLayer>() || layer.is<ImageLayer>(), "Layer must be TextLayer or ImageLayer");
    const sp<ImageLayer> imageLayer = layer.is<ImageLayer>() ? layer.as<ImageLayer>() : layer.as<AlphabetLayer>()->imageLayer();
    return sp<FPSCounter>::make(Ark::instance().clock(), _interval, imageLayer, _message);
}

}
