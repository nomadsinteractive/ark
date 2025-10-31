#pragma once

#include <utility>

#include "core/base/api.h"

namespace ark {

class ARK_API UpdatableUtil {
public:
    template<typename T, typename... Args> static bool update(const uint32_t tick, T& var, Args&&... vars) {
        const bool dirty = update_sfinae(var, tick, nullptr);
        if constexpr(sizeof...(vars) > 0)
            return update(tick, std::forward<Args>(vars)...) || dirty;
        else
            return dirty;
    }

private:
    template<typename T> static bool update_sfinae(T& var, const uint32_t tick, decltype(var->update(tick))*) {
        return var ? var->update(tick) : false;
    }

    template<typename T> static bool update_sfinae(T& var, const uint32_t tick, decltype(var.update(tick))*) {
        return var.update(tick);
    }
};

}
