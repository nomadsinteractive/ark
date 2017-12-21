#ifndef ARK_APP_IMPL_COLLIDER_TILED_COLLIDER_H_
#define ARK_APP_IMPL_COLLIDER_TILED_COLLIDER_H_

#include <set>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/base/rect.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"
#include "app/base/rigid_body.h"

namespace ark {

class TiledCollider : public Collider {
public:
    TiledCollider(const sp<TileMap>& tileMap, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size) override;

//  [[plugin::resource-loader("tiled")]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        sp<Builder<TileMap>> _tile_map;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

    class RigidBodyImpl;

public:
    class RigidBodyImpl : public RigidBody {
    public:
        struct Stub {
            Stub(const sp<TileMap>& tileMap, const sp<VV2>& position);

            void collision(const Rect& rect);

            sp<VV2> _position;
            sp<CollisionCallback> _collision_callback;

        private:
            void beginContact(const sp<RigidBodyImpl>& rigidBody);
            void endContact(const sp<RigidBodyImpl>& rigidBody);

        private:
            sp<TileMap> _tile_map;
            std::set<uint32_t> _contacts;
            sp<RigidBodyImpl> _static_rigid_body;
            Rect _last_collision;
        };

    public:
        RigidBodyImpl(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<TileMap>& tileMap);

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        void setId(uint32_t id);
        void setPosition(const sp<VV>& position);

        V2 xy() const;

        void collision(const Rect& rect);

        const sp<Stub>& stub() const;

    private:
        sp<Stub> _stub;
    };

private:
    sp<TileMap> _tile_map;
    sp<ResourceLoaderContext> _resource_loader_context;

    uint32_t _rigid_body_base;
};

}

#endif
