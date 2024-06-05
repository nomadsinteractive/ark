#pragma once

#include <utility>

#include "core/base/api.h"

namespace ark {

class ARK_API UpdatableUtil {
public:
    template<typename T, typename... Args> static bool update(uint64_t timestamp, T& var, Args&&... vars) {
        bool dirty = update_sfinae(var, timestamp, nullptr);
        if constexpr(sizeof...(vars) > 0)
            return update(timestamp, std::forward<Args>(vars)...) || dirty;
        else
            return dirty;
    }

private:
    template<typename T> static bool update_sfinae(T& var, uint64_t timestamp, decltype(var->update(timestamp))*) {
        return var ? var->update(timestamp) : false;
    }

    template<typename T> static bool update_sfinae(T& var, uint64_t timestamp, decltype(var.update(timestamp))*) {
        return var.update(timestamp);
    }
};

}
