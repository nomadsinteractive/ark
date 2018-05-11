#ifndef ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_
#define ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_

#include <unordered_map>

#include "app/inf/tracker.h"

namespace ark {

class TrackerGrid : public Tracker {
public:
    virtual sp<VV> create(int32_t id, const sp<VV>& position, const sp<VV>& aabb) override;
    virtual void remove(int32_t id) override;
    virtual std::unordered_set<int32_t> search(const V& position, const V& aabb) override;

private:
    class Axis {
    public:
        void update(int32_t id, float pos, float radius);
        std::unordered_set<int32_t> search(float pos, float radius);

    private:
        std::unordered_multimap<int32_t, int32_t> _trackee_grids;


    };

};

}

#endif
