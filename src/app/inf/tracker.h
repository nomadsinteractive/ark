#ifndef ARK_APP_INF_TRACKER_H_
#define ARK_APP_INF_TRACKER_H_

#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Tracker {
public:
    virtual ~Tracker() = default;

//  [[script::bindings::auto]]
    virtual sp<Vec> create(int32_t id, const sp<Vec>& position, const sp<Vec>& aabb) = 0;
//  [[script::bindings::auto]]
    virtual void remove(int32_t id) = 0;

//  [[script::bindings::auto]]
    virtual std::unordered_set<int32_t> search(const V& position, const V& aabb) = 0;
};

}

#endif
