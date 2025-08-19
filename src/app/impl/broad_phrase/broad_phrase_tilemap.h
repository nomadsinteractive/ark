#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/inf/broad_phrase.h"

namespace ark {

class BroadPhraseTilemap final : public BroadPhrase {
public:
    BroadPhraseTilemap(sp<Tilemap> tilemap);

    sp<Coordinator> requestCoordinator() override;

    Result search(const V3& position, const V3& size) override;
    Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

//  [[plugin::builder("broad-phrase-tilemap")]]
    class BUILDER final : public Builder<BroadPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<BroadPhrase> build(const Scope& args) override;

    private:
        sp<Builder<Tilemap>> _tilemap;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
    };

private:
    sp<Tilemap> _tilemap;
};

}
