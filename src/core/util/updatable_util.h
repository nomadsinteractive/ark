#ifndef ARK_CORE_UTIL_UPDATABLE_UTIL_H_
#define ARK_CORE_UTIL_UPDATABLE_UTIL_H_

#include <utility>

#include "core/base/api.h"

namespace ark {

class ARK_API UpdatableUtil {
public:
    template<typename T, typename... VARS> static bool update(uint64_t timestamp, T& var, VARS&&... vars) {
        bool dirty = update_sfinae(var, timestamp, nullptr);
        return update(timestamp, std::forward<VARS>(vars)...) || dirty;
    }

private:

    static bool update(uint64_t /*timestamp*/) {
        return false;
    }

    template<typename T> static bool update_sfinae(T& var, uint64_t timestamp, decltype(var->update(timestamp))*) {
        return var->update(timestamp);
    }

    template<typename T> static bool update_sfinae(T& var, uint64_t timestamp, decltype(var.update(timestamp))*) {
        return var.update(timestamp);
    }

    template<typename T> static bool update_sfinae(T& /*var*/, uint64_t /*timestamp*/, ...) {
        DFATAL("Unable to update this variable");
        return true;
    }

};

}

#endif
