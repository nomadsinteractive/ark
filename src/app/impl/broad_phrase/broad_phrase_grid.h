#ifndef ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_GRID_H_
#define ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_GRID_H_

#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/updatable_util.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseGrid : public BroadPhrase {
public:
    BroadPhraseGrid(int32_t dimension, const V3& cell);

    virtual void create(int32_t id, const V3& position, const V3& aabb) override;
    virtual void update(int32_t id, const V3& position, const V3& aabb) override;
    virtual void remove(int32_t id) override;

    virtual Result search(const V3& position, const V3& size) override;
    virtual Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

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
        Stub(int32_t dimension, const V3& cell);
        ~Stub();

        void remove(int32_t id);
        void create(int32_t id, const V3& position, const V3& size);
        void update(int32_t id, const V3& position, const V3& size);

        std::unordered_set<int32_t> search(const V3& position, const V3& size) const;

    private:
        int32_t _dimension;
        Axis* _axes;
    };

//  [[plugin::builder("broad-phrase-grid")]]
    class BUILDER : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<BroadPhrase> build(const Scope& args) override;

    private:
        int32_t _dimension;
        sp<Builder<Vec3>> _cell;

    };

private:
    sp<Stub> _stub;
};

}

#endif
