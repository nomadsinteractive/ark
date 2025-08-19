#pragma once

#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseGrid final : public BroadPhrase {
public:
    BroadPhraseGrid(int32_t dimension, const V3& cell);

    sp<Coordinator> requestCoordinator() override;

    Result search(const V3& position, const V3& size) override;
    Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

    class Stub;

    class Axis {
    public:
        void create(CandidateIdType id, float position, float low, float high);
        void update(CandidateIdType id, float position, float low, float high);
        void remove(CandidateIdType id);

        HashSet<CandidateIdType> search(float low, float high) const;

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

        void updateRange(CandidateIdType id, const Range& cur, const Range& prev);
        void remove(CandidateIdType id, int32_t rangeId);

    private:
        HashMap<CandidateIdType, Range> _trackee_ranges;
        std::unordered_multimap<int32_t, CandidateIdType> _trackee_range_ids;
        int32_t _stride;

        friend class Stub;
    };

//  [[plugin::builder("broad-phrase-grid")]]
    class BUILDER final : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<BroadPhrase> build(const Scope& args) override;

    private:
        int32_t _dimension;
        sp<Builder<Vec3>> _cell;

    };

private:
    sp<Stub> _stub;
};

}
