#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseTrie final : public BroadPhrase {
public:
    BroadPhraseTrie(int32_t dimension);

    sp<Coordinator> requestCoordinator() override;

    void search(BroadPhraseCallback& callback, V3 position, V3 size) override;
    void rayCast(BroadPhraseCallback& callback, V3 from, V3 to, const sp<CollisionFilter>& collisionFilter) override;

public:
    class Stub;

    class Axis {
    private:
        struct Boundary {
            int32_t key;
            Set<RefId> items;
        };

        struct Range {
            Range();
            Range(Boundary* lower, Boundary* upper);

            Boundary* _lower;
            Boundary* _upper;
        };

    public:
        void create(RefId id, float low, float high);
        void update(RefId id, float low, float high);
        void remove(RefId id);

        HashSet<RefId> search(float low, float high) const;

    private:
        Boundary* boundaryCreate(Map<int32_t, Boundary>& boundaries, RefId id, int32_t key);
        Boundary* boundaryUpdate(Map<int32_t, Boundary>& boundaries, Boundary* boundary, int32_t key, RefId id);
        void boundaryRemove(Map<int32_t, Boundary>& boundaries, Boundary* boundary, RefId id);

        Range& ensureRange(int32_t id);

    private:
        Map<int32_t, Boundary> _lower_bounds;
        Map<int32_t, Boundary> _upper_bounds;

        HashMap<int32_t, Range> _ranges;
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
