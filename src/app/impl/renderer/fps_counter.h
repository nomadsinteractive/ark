#pragma once

#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API FPSCounter final : public StringVar {
public:
//  [[script::bindings::constructor]]
    FPSCounter(String messageFormat = "%.1f FPS", float refreshInterval = 2.0f);

    bool update(uint32_t tick) override;
    StringView val() override;

private:
    String _format;
    String _value;

    uint32_t _frame_rendered;
    uint32_t _last_tick;
    uint32_t _last_updated_tick;
    sp<Numeric> _duration;
    float _refresh_interval;
    float _last_duration;
};

}
