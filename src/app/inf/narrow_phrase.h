#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/broad_phrase.h"

namespace ark {

class ARK_API NarrowPhrase {
public:
    class Ray {
    public:
        template<class T> const T* data() const {
            return reinterpret_cast<const T*>(_data);
        }

        template<class T> T* data() {
            return reinterpret_cast<T*>(_data);
        }

    private:
        int32_t _data[8];
    };

public:
    virtual ~NarrowPhrase() = default;

    virtual RigidbodyDef makeBodyDef(TypeId shapeId, const SafeVar<Vec3>& size) = 0;

    virtual Ray toRay(const V2& from, const V2& to) = 0;

    virtual bool collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold) = 0;
    virtual Optional<RayCastManifold> rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate) = 0;
};

}
