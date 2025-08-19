#pragma once

#include "core/inf/builder.h"

#include "graphics/forwarding.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseAll final : public BroadPhrase {
public:
    BroadPhraseAll();

    sp<Coordinator> requestCoordinator() override;

    Result search(BroadPhraseCallback& callback, V3 position, V3 size) override;
    Result rayCast(BroadPhraseCallback& callback, V3 from, V3 to, const sp<CollisionFilter>& collisionFilter) override;

//  [[plugin::builder("broad-phrase-all")]]
    class BUILDER final : public Builder<BroadPhrase> {
    public:
        BUILDER() = default;

        sp<BroadPhrase> build(const Scope& args) override;
    };

private:
    class Stub;

    sp<Stub> _stub;
};

}
