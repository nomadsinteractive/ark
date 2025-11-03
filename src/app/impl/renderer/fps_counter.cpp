#include "app/impl/renderer/fps_counter.h"

#include "core/ark.h"
#include "core/inf/variable.h"
#include "core/base/clock.h"
#include "core/util/strings.h"

namespace ark {

FPSCounter::FPSCounter(String messageFormat, const float refreshInterval)
    : _format(std::move(messageFormat)), _frame_rendered(0), _last_tick(0), _last_updated_tick(0), _duration(Ark::instance().sysClock()->duration()), _refresh_interval(refreshInterval), _last_duration(0)
{
}

bool FPSCounter::update(const uint32_t tick)
{
    if(_last_tick != tick)
    {
        const float duration = _duration->val();
        const float elapsed = duration - _last_duration;
        ++ _frame_rendered;
        if(elapsed > _refresh_interval)
        {
            if(_last_duration != 0)
                _value = Strings::sprintf(_format.c_str(), _frame_rendered / elapsed);
            _last_duration = duration;
            _frame_rendered = 0;
            _last_updated_tick = tick;
        }
        _last_tick = tick;
    }
    return _last_updated_tick == tick;
}

StringView FPSCounter::val()
{
    return _value;
}

}
