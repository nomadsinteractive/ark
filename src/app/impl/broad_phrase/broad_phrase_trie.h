#ifndef ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_TRIE_H_
#define ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_TRIE_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/util/variable_util.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseTrie : public BroadPhrase {
public:
    BroadPhraseTrie(int32_t dimension);

    virtual void create(int32_t id, const V3& position, const V3& aabb) override;
    virtual void update(int32_t id, const V3& position, const V3& aabb) override;
    virtual void remove(int32_t id) override;

    virtual Result search(const V3& position, const V3& size) override;
    virtual Result rayCast(const V3& from, const V3& to) override;

public:
    class Stub;

    class Axis {
    private:
        struct Boundary {
            int32_t key;
            std::set<int32_t> items;
        };

        struct Range {
            Range();
            Range(Boundary* lower, Boundary* upper);

            Boundary* _lower;
            Boundary* _upper;
        };

    public:
        void create(int32_t id, float low, float high);
        void update(int32_t id, float low, float high);
        void remove(int32_t id);

        std::unordered_set<int32_t> search(float low, float high) const;

    private:
        Boundary* boundaryCreate(std::map<int32_t, Boundary>& boundaries, int32_t id, int32_t key);
        Boundary* boundaryUpdate(std::map<int32_t, Boundary>& boundaries, Boundary* boundary, int32_t key, int32_t id);
        void boundaryRemove(std::map<int32_t, Boundary>& boundaries, Boundary* boundary, int32_t id);

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

        void remove(int32_t id);
        void create(int32_t id, const V3& position, const V3& size);
        void update(int32_t id, const V3& position, const V3& size);

        std::unordered_set<int32_t> search(const V3& position, const V3& size) const;

    private:
        int32_t _dimension;
        Axis* _axes;
    };

//  [[plugin::builder("broad-phrase-trie")]]
    class BUILDER : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<BroadPhrase> build(const Scope& args) override;

    private:
        int32_t _dimension;

    };

private:
    sp<Stub> _stub;
};

}

#endif
