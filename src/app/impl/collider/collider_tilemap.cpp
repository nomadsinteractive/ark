#include "app/impl/collider/collider_tilemap.h"

#include "core/base/bean_factory.h"
#include "core/types/null.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/tilemap.h"
#include "graphics/base/tilemap_layer.h"
#include "graphics/base/tileset.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"
#include "app/base/rigid_body.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class DynamicPosition : public Vec3 {
public:
    DynamicPosition(const sp<ColliderTilemap::RigidBodyImpl>& rigidBody, const sp<Vec3>& delegate)
        : _rigid_body(rigidBody), _delegate(delegate), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height()) {
    }

    virtual V3 val() override {
        const V3 position = _delegate->val();
        const sp<ColliderTilemap::RigidBodyImpl> rigidBody = _rigid_body.lock();
        if(rigidBody) {
            _bounds.setCenter(position.x(), position.y());
            rigidBody->collision(_bounds);
        }
        return position;
    }

    virtual bool update(uint64_t timestamp) override {
        return _delegate->update(timestamp);
    }

private:
    WeakPtr<ColliderTilemap::RigidBodyImpl> _rigid_body;
    sp<Vec3> _delegate;
    Rect _bounds;

};

}

ColliderTilemap::ColliderTilemap(const sp<Tilemap>& tilemap, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tilemap(tilemap), _resource_loader_context(resourceLoaderContext), _rigid_body_base(0)
{
}

ColliderTilemap::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tilemap(factory.ensureBuilder<Tilemap>(manifest, "tilemap")), _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> ColliderTilemap::BUILDER::build(const Scope& args)
{
    return sp<ColliderTilemap>::make(_tilemap->build(args), _resource_loader_context);
}

sp<RigidBody> ColliderTilemap::createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& /*rotate*/)
{
    DCHECK(type != Collider::BODY_TYPE_STATIC, "Cannot create static body in ColliderTilemap");
    DASSERT(position && size);

    const sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(++_rigid_body_base, type, position, size, _tilemap);
    rigidBody->stub()->_position = _resource_loader_context->synchronize<V3>(sp<DynamicPosition>::make(rigidBody, position));
    return rigidBody;
}

std::vector<RayCastManifold> ColliderTilemap::rayCast(const V3& from, const V3& to)
{
    DFATAL("Unimplemented");
    return {};
}

ColliderTilemap::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec3>& position, const sp<Size>& size, const sp<Tilemap>& tileMap)
    : RigidBody(id, type, position, size, Null::toSafe<Rotation>(nullptr), Box(), nullptr), _tilemap(tileMap), _rigid_body_shadow(sp<RigidBodyShadow>::make(tileMap->tileset()->tileWidth(), tileMap->tileset()->tileHeight()))
{
}

void ColliderTilemap::RigidBodyImpl::updateRigidBodyShadow(const Contact& contact)
{
    _rigid_body_shadow->setId(contact._id);
    _rigid_body_shadow->_position->set(V3(contact._position, 0));
    _rigid_body_shadow->stub()->_render_object = contact._render_object;
}

void ColliderTilemap::RigidBodyImpl::dispose()
{
}

void ColliderTilemap::RigidBodyImpl::collision(const Rect& aabb)
{
    if(!callback()->hasCallback())
        return;

    std::set<Contact> candidates = _contacts;
    std::set<Contact> contacts;

    uint32_t layerId = 0;
    for(const sp<TilemapLayer>& i : _tilemap->layers())
    {
        if(i->flag() & Tilemap::LAYER_FLAG_COLLIDABLE)
        {
            V3 selectionPoint;
            RectI selectionRange;
            if(i->getSelectionTileRange(aabb, selectionPoint, selectionRange))
            {
                const V3 position = i->position()->val();
                float tileWidth = static_cast<float>(i->tileset()->tileWidth());
                float tileHeight = static_cast<float>(i->tileset()->tileHeight());

                uint32_t colCount = i->colCount();
                int32_t bColId = selectionRange.left();
                int32_t eColId = selectionRange.right() - 1;
                int32_t bRowId = selectionRange.top();
                int32_t eRowId = selectionRange.bottom() - 1;

                for(int32_t col = bColId; col <= eColId; col ++)
                    for(int32_t row = bRowId; row <= eRowId; row ++)
                    {
                        const sp<RenderObject>& tile = i->getTile(row, col);
                        if(tile)
                        {
                            const V3 contactPoint = position + V3((col + 0.5f) * tileWidth, (row + 0.5f) * tileHeight, 0);
                            const Contact contact(layerId, row, col, colCount, contactPoint, tile);
                            candidates.insert(contact);
                            contacts.insert(contact);

                            bool notInContacts = _contacts.find(contact) == _contacts.end();
                            if(notInContacts)
                            {
                                updateRigidBodyShadow(contact);
                                const V3 normal(col == bColId || col == eColId ? (aabb.left() + aabb.right() > contactPoint.x() * 2 ? 1.0f : -1.0f) : 0.0f,
                                                row == bRowId || row == eRowId ? (aabb.top() + aabb.bottom() > contactPoint.y() * 2 ? 1.0f : -1.0f) : 0.0f,
                                                0);
                                if(callback()->hasCallback())
                                    callback()->onBeginContact(_rigid_body_shadow, CollisionManifold(contactPoint, normal));
                            }
                        }
                    }
            }
        }
        ++layerId;
    }

    for(const Contact& contact : candidates)
        if(contacts.find(contact) == contacts.end())
        {
            updateRigidBodyShadow(contact);
            callback()->onEndContact(_rigid_body_shadow);
        }
    _contacts = contacts;
}

ColliderTilemap::RigidBodyShadow::RigidBodyShadow(uint32_t width, uint32_t height)
    : RigidBody(0, Collider::BODY_TYPE_STATIC, sp<Vec3::Impl>::make(V3()), sp<Size>::make(static_cast<float>(width), static_cast<float>(height)), nullptr, Box(), nullptr)
{
    _position = position();
}

void ColliderTilemap::RigidBodyShadow::setId(uint32_t id)
{
    stub()->_id = id;
}

void ColliderTilemap::RigidBodyShadow::dispose()
{
}

ColliderTilemap::Contact::Contact(uint32_t layerId, uint32_t row, uint32_t col, uint32_t colCount, const V2& position, const sp<RenderObject>& renderObject)
    : _id((layerId << 24) + (row * colCount + col)), _position(position), _render_object(renderObject)
{
}

bool ColliderTilemap::Contact::operator <(const ColliderTilemap::Contact& other) const
{
    return _id < other._id;
}

}
