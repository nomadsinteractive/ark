#ifndef ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_
#define ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_

#include <unordered_map>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/tracker.h"

namespace ark {

class TrackerGrid : public Tracker {
public:
    TrackerGrid(const V& cell);

    virtual sp<Vec> create(int32_t id, const sp<Vec>& position, const sp<Vec>& size) override;
    virtual void remove(int32_t id) override;

    virtual std::unordered_set<int32_t> search(const V& position, const V& size) override;

public:
    class Stub;

    class Axis {
    public:
        void create(int32_t id, float position, float low, float high);
        void update(int32_t id, float position, float low, float high);
        void remove(int32_t id);

        std::unordered_set<int32_t> search(float low, float high) const;

    private:
        struct Range {
            Range();
            Range(int32_t position, int32_t begin, int32_t end);
            Range(const Range& other) = default;

            bool within(int32_t r) const;

            int32_t _position;
            int32_t _begin;
            int32_t _end;
        };

        void updateRange(int32_t id, const Range& cur, const Range& prev);
        void remove(int32_t id, int32_t rangeId);

    private:
        std::unordered_map<int32_t, Range> _trackee_ranges;
        std::unordered_multimap<int32_t, int32_t> _trackee_range_ids;
        int32_t _stride;

        friend class Stub;
    };

    class Stub {
    public:
        Stub(const V& cell);

        void remove(int32_t id);
        void create(int32_t id, const V& position, const V& size);
        void update(int32_t id, const V& position, const V& size);

        std::unordered_set<int32_t> search(const V& position, const V& size) const;

    private:
        Axis _axes[DIMENSIONS];
    };

//  [[plugin::builder("grid")]]
    class BUILDER : public Builder<Tracker> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Tracker> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Vec>> _cell;

    };

private:
    sp<Stub> _stub;
};

}

#endif
