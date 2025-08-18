#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseTrie : public BroadPhrase {
public:
    BroadPhraseTrie(int32_t dimension);

    void create(CandidateIdType id, const V3& position, const V3& aabb) override;
    void update(CandidateIdType id, const V3& position, const V3& aabb) override;
    void remove(CandidateIdType id) override;

    Result search(const V3& position, const V3& size) override;
    Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

public:
    class Stub;

    class Axis {
    private:
        struct Boundary {
            int32_t key;
            Set<CandidateIdType> items;
        };

        struct Range {
            Range();
            Range(Boundary* lower, Boundary* upper);

            Boundary* _lower;
            Boundary* _upper;
        };

    public:
        void create(CandidateIdType id, float low, float high);
        void update(CandidateIdType id, float low, float high);
        void remove(CandidateIdType id);

        HashSet<CandidateIdType> search(float low, float high) const;

    private:
        Boundary* boundaryCreate(std::map<int32_t, Boundary>& boundaries, CandidateIdType id, int32_t key);
        Boundary* boundaryUpdate(std::map<int32_t, Boundary>& boundaries, Boundary* boundary, int32_t key, CandidateIdType id);
        void boundaryRemove(std::map<int32_t, Boundary>& boundaries, Boundary* boundary, CandidateIdType id);

        Range& ensureRange(int32_t id);

    private:
        std::map<int32_t, Boundary> _lower_bounds;
        std::map<int32_t, Boundary> _upper_bounds;

        std::unordered_map<int32_t, Range> _ranges;
    };

    class Stub {
    public:
        Stub(int32_t dimension);
        ~Stub();

        void remove(CandidateIdType id);
        void create(CandidateIdType id, const V3& position, const V3& size);
        void update(CandidateIdType id, const V3& position, const V3& size);

        HashSet<CandidateIdType> search(const V3& position, const V3& size) const;

    private:
        int32_t _dimension;
        Axis* _axes;
    };

//  [[plugin::builder("broad-phrase-trie")]]
    class BUILDER final : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<BroadPhrase> build(const Scope& args) override;

    private:
        int32_t _dimension;

    };

private:
    sp<Stub> _stub;
};

}
