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
#include "app/base/rigid_body.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class DynamicPosition : public Vec2 {
public:
    DynamicPosition(const sp<TiledCollider::RigidBodyImpl>& rigidBody, const sp<Vec>& delegate)
        : _rigid_body(rigidBody), _delegate(delegate), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height()) {
    }

    virtual V2 val() override {
        const V2 position = _delegate->val();
        const sp<TiledCollider::RigidBodyImpl> rigidBody = _rigid_body.lock();
        if(rigidBody) {
            _bounds.setCenter(position.x(), position.y());
            rigidBody->collision(_bounds);
        }
        return position;
    }

private:
    WeakPtr<TiledCollider::RigidBodyImpl> _rigid_body;
    sp<Vec> _delegate;
    Rect _bounds;
};

}

TiledCollider::TiledCollider(const sp<Tilemap>& tilemap, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tilemap(tilemap), _resource_loader_context(resourceLoaderContext), _rigid_body_base(0)
{
}

TiledCollider::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tilemap(factory.ensureBuilder<Tilemap>(manifest, "tilemap")), _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> TiledCollider::BUILDER::build(const Scope& args)
{
    return sp<TiledCollider>::make(_tilemap->build(args), _resource_loader_context);
}

sp<RigidBody> TiledCollider::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& /*rotate*/)
{
    DCHECK(type != Collider::BODY_TYPE_STATIC, "Cannot create static body in TiledCollider");
    DASSERT(position && size);

    const sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(++_rigid_body_base, type, position, size, _tilemap);
    rigidBody->stub()->_position = _resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody, position));
    return rigidBody;
}

TiledCollider::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Tilemap>& tileMap)
    : RigidBody(id, type, position, size, Null::toSafe<Rotate>(nullptr)), _tilemap(tileMap), _rigid_body_shadow(sp<RigidBodyShadow>::make(tileMap->tileset()->tileWidth(), tileMap->tileset()->tileHeight()))
{
}

void TiledCollider::RigidBodyImpl::updateRigidBodyShadow(const Contact& contact)
{
    _rigid_body_shadow->setId(contact._id);
    _rigid_body_shadow->_position->set(contact._position);
    _rigid_body_shadow->stub()->_render_object = contact._render_object;
}

void TiledCollider::RigidBodyImpl::dispose()
{
}

void TiledCollider::RigidBodyImpl::collision(const Rect& rect)
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
            const V position = i->position()->val();
            float left = rect.left() - position.x();
            float right = rect.right() - position.x();
            float top = rect.top() - position.y();
            float bottom = rect.bottom() - position.y();
            float tileWidth = static_cast<float>(i->tileset()->tileWidth());
            float tileHeight = static_cast<float>(i->tileset()->tileHeight());

            int32_t bColId = static_cast<int32_t>(left / tileWidth) - (left < 0 ? 1 : 0);
            int32_t eColId = static_cast<int32_t>(right / tileWidth) - (right < 0 ? 1 : 0);
            int32_t bRowId = static_cast<int32_t>(top / tileHeight) - (top < 0 ? 1 : 0);
            int32_t eRowId = static_cast<int32_t>(bottom / tileHeight) - (bottom < 0 ? 1 : 0);

            uint32_t colCount = i->colCount();
            uint32_t rowCount = i->rowCount();

            for(int32_t col = bColId; col <= eColId; col ++)
                for(int32_t row = bRowId; row <= eRowId; row ++)
                    if(col >= 0 && row >= 0 && col < colCount && row < rowCount)
                    {
                        const sp<RenderObject>& tile = i->getTile(row, col);
                        if(tile)
                        {
                            const Contact contact(layerId, row, col, colCount, position + V2((col + 0.5f) * tileWidth, (row + 0.5f) * tileHeight), tile);
                            candidates.insert(contact);
                            contacts.insert(contact);

                            bool notInContacts = _contacts.find(contact) == _contacts.end();
                            if(notInContacts)
                            {
                                updateRigidBodyShadow(contact);
                                const V normal(col == bColId ? 1.0f : (col == eColId ? -1.0f : 0.0f),
                                               row == bRowId ? 1.0f : (row == eRowId ? -1.0f : 0.0f));
                                if(callback()->hasCallback())
                                    callback()->onBeginContact(_rigid_body_shadow, CollisionManifold(normal));
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

TiledCollider::RigidBodyShadow::RigidBodyShadow(uint32_t width, uint32_t height)
    : RigidBody(0, Collider::BODY_TYPE_STATIC, sp<Vec2::Impl>::make(V2()), sp<Size>::make(static_cast<float>(width), static_cast<float>(height)), nullptr)
{
    _position = position();
}

void TiledCollider::RigidBodyShadow::setId(uint32_t id)
{
    stub()->_id = id;
}

void TiledCollider::RigidBodyShadow::dispose()
{
}

TiledCollider::Contact::Contact(uint32_t layerId, uint32_t row, uint32_t col, uint32_t colCount, const V2& position, const sp<RenderObject>& renderObject)
    : _id((layerId << 24) + (row * colCount + col)), _position(position), _render_object(renderObject)
{
}

bool TiledCollider::Contact::operator <(const TiledCollider::Contact& other) const
{
    return _id < other._id;
}

}
