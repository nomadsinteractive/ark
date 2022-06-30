#ifndef ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_ALL_H_
#define ARK_APP_IMPL_BROAD_PHRASE_BROAD_PHRASE_ALL_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseAll : public BroadPhrase {
public:
    BroadPhraseAll();

    virtual void create(int32_t id, const V3& position, const V3& aabb) override;
    virtual void update(int32_t id, const V3& position, const V3& aabb) override;
    virtual void remove(int32_t id) override;

    virtual Result search(const V3& position, const V3& size) override;
    virtual Result rayCast(const V3& from, const V3& to) override;

//  [[plugin::builder("broad-phrase-all")]]
    class BUILDER : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<BroadPhrase> build(const Scope& args) override;

    };

private:
    std::unordered_set<int32_t> _candidates;
};

}

#endif
