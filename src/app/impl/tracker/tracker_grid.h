#ifndef ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_
#define ARK_APP_IMPL_TRACKER_TRACKER_GRID_H_

#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/variable_util.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/tracker.h"

namespace ark {

class TrackerGrid : public Tracker {
public:
    TrackerGrid(uint32_t dimension, const V3& cell);

    virtual sp<Vec3> create(int32_t id, const sp<Vec3>& position, const sp<Vec3>& size) override;
    virtual void remove(int32_t id) override;

    virtual std::unordered_set<int32_t> search(const V3& position, const V3& size) override;

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
        Stub(uint32_t dimension, const V3& cell);
        ~Stub();

        void remove(int32_t id);
        void create(int32_t id, const V3& position, const V3& size);
        void update(int32_t id, const V3& position, const V3& size);

        std::unordered_set<int32_t> search(const V3& position, const V3& size) const;

    private:
        uint32_t _dimension;
        Axis* _axes;
    };

//  [[plugin::builder("grid")]]
    class BUILDER : public Builder<Tracker> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Tracker> build(const Scope& args) override;

    private:
        uint32_t _dimension;
        sp<Builder<Vec3>> _cell;

    };

private:
    class TrackedPosition : public Vec3 {
    public:
        TrackedPosition(int32_t id, const sp<TrackerGrid::Stub>& stub, const sp<Vec3>& position, const sp<Vec3>& size)
            : _id(id), _stub(stub), _position(position), _size(size) {
            const V3 p = _position->val();
            const V3 s = _size->val();
            _stub->create(_id, p, s);
        }
        ~TrackedPosition() override {
            _stub->remove(_id);
        }

        virtual V3 val() override {
            const V3 p = _position->val();
            const V3 s = _size->val();
            _stub->update(_id, p, s);
            return p;
        }

        virtual bool update(uint64_t timestamp) override {
            return VariableUtil::update(timestamp, _position, _size);
        }

    private:
        int32_t _id;
        sp<TrackerGrid::Stub> _stub;
        sp<Vec3> _position;
        sp<Vec3> _size;
    };

private:
    sp<Stub> _stub;
};

}

#endif
