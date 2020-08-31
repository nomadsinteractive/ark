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

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Quaternion>& rotate) override;

//  [[plugin::resource-loader("tilemap")]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        sp<Builder<Tilemap>> _tilemap;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t width, uint32_t height);

        void setId(uint32_t id);

        virtual void dispose() override;

        sp<Vec3::Impl> _position;
    };

public:

    struct Contact {
        Contact(uint32_t layerId, uint32_t row, uint32_t col, uint32_t colCount, const V2& position, const sp<RenderObject>& renderObject);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Contact);

        uint32_t _id;
        V2 _position;
        sp<RenderObject> _render_object;

        bool operator <(const Contact& other) const;
    };

    class RigidBodyImpl : public RigidBody {
    public:
        RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Tilemap>& tileMap);

        virtual void dispose() override;

        void collision(const Rect& rect);

    private:
        void updateRigidBodyShadow(const Contact& contact);

    private:
        sp<Tilemap> _tilemap;
        sp<RigidBodyShadow> _rigid_body_shadow;

        std::set<Contact> _contacts;
    };

private:
    sp<Tilemap> _tilemap;
    sp<ResourceLoaderContext> _resource_loader_context;

    uint32_t _rigid_body_base;
};

}

#endif
