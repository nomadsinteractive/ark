#pragma once

#include <iterator>
#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API BroadPhrase {
public:
    struct Candidate {
        RefId _id;
        V3 _position;
        V4 _quaternion;
        sp<Shape> _shape;
        sp<CollisionFilter> _collision_filter;
    };

    class Coordinator {
    public:
        virtual ~Coordinator() = default;

        virtual void create(RefId id, const V3& position, const V3& size) = 0;
        virtual void update(RefId id, const V3& position, const V3& size) = 0;
        virtual void remove(RefId id) = 0;
    };

public:
    virtual ~BroadPhrase() = default;

    virtual sp<Coordinator> requestCoordinator() = 0;

    virtual void search(BroadPhraseCallback& callback, V3 position, V3 size) = 0;
    virtual void rayCast(BroadPhraseCallback& callback, V3 from, V3 to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;
};

}
