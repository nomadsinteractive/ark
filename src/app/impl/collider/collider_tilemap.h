#ifndef ARK_APP_IMPL_COLLIDER_COLLIDER_TILED_H_
#define ARK_APP_IMPL_COLLIDER_COLLIDER_TILED_H_

#include <set>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
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
    TiledCollider(const sp<Tilemap>& tilemap, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate) override;

//  [[plugin::resource-loader("tilemap")]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Tilemap>> _tilemap;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t width, uint32_t height);

        void setId(uint32_t id);
        void setPosition(float x, float y);

        virtual void dispose() override;

    private:
        sp<Vec2::Impl> _position;
    };

public:
    class RigidBodyImpl : public RigidBody {
    public:
        RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Tilemap>& tileMap);

        virtual void dispose() override;

        void collision(const Rect& rect);

    private:
        void updateRigidBodyShadow(uint32_t id, float tileWidth, float tileHeight, uint32_t colCount, const sp<RenderObject>& renderObject);

    private:
        sp<Tilemap> _tilemap;
        sp<RigidBodyShadow> _rigid_body_shadow;

        std::set<uint32_t> _contacts;
    };

private:
    sp<Tilemap> _tilemap;
    sp<ResourceLoaderContext> _resource_loader_context;

    uint32_t _rigid_body_base;
};

}

#endif
