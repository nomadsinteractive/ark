#include "app/impl/collider/collider_tiled.h"

#include "core/base/bean_factory.h"
#include "core/types/null.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/rect.h"
#include "graphics/base/render_object.h"
#include "graphics/base/size.h"
#include "graphics/base/v2.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/collision_manifold.h"
#include "app/base/rigid_body.h"
#include "app/base/tile_map.h"
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
        _bounds.setCenter(position.x(), position.y());
        _rigid_body->collision(_bounds);
        return position;
    }

private:
    sp<TiledCollider::RigidBodyImpl> _rigid_body;
    sp<Vec> _delegate;
    Rect _bounds;
};

}

TiledCollider::TiledCollider(const sp<TileMap>& tileMap, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tile_map(tileMap), _resource_loader_context(resourceLoaderContext), _rigid_body_base(0)
{
}

TiledCollider::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _tile_map(factory.ensureBuilder<TileMap>(manifest, "tile-map")), _resource_loader_context(resourceLoaderContext)
{
}

sp<Collider> TiledCollider::BUILDER::build(const sp<Scope>& args)
{
    return sp<TiledCollider>::make(_tile_map->build(args), _resource_loader_context);
}

sp<RigidBody> TiledCollider::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotation>& /*rotate*/)
{
    DCHECK(type != Collider::BODY_TYPE_STATIC, "Cannot create static body in TiledCollider");
    DASSERT(position && size);

    const sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(++_rigid_body_base, type, position, size, _tile_map);
    rigidBody->stub()->_position = _resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody, position));
    return rigidBody;
}

TiledCollider::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<TileMap>& tileMap)
    : RigidBody(id, type, position, size, Null::ptr<Rotation>()), _tile_map(tileMap), _rigid_body_tile(sp<RigidBodyTile>::make(tileMap->tileWidth(), tileMap->tileHeight()))
{
}

void TiledCollider::RigidBodyImpl::updateRigidBodyTile(uint32_t id, float tileWidth, float tileHeight, uint32_t colCount)
{
    uint32_t row = id / colCount;
    uint32_t col = id % colCount;
    _rigid_body_tile->setId(id);
    _rigid_body_tile->setPosition(col * tileWidth + tileWidth / 2.0f, row * tileHeight + tileHeight / 2.0f);
}

void TiledCollider::RigidBodyImpl::dispose()
{
}

void TiledCollider::RigidBodyImpl::collision(const Rect& rect)
{
    if(!callback()->hasCallback())
        return;

    const V position = _tile_map->position()->val();
    float left = rect.left() - position.x();
    float right = rect.right() - position.x();
    float top = rect.top() - position.y();
    float bottom = rect.bottom() - position.y();
    std::set<uint32_t> candidates = _contacts;
    std::set<uint32_t> contacts;
    float tileWidth = static_cast<float>(_tile_map->tileWidth());
    float tileHeight = static_cast<float>(_tile_map->tileHeight());

    int32_t bColId = static_cast<int32_t>(left / _tile_map->tileWidth()) - (left < 0 ? 1 : 0);
    int32_t eColId = static_cast<int32_t>(right / _tile_map->tileWidth()) - (right < 0 ? 1 : 0);
    int32_t bRowId = static_cast<int32_t>(top / _tile_map->tileHeight()) - (top < 0 ? 1 : 0);
    int32_t eRowId = static_cast<int32_t>(bottom / _tile_map->tileHeight()) - (bottom < 0 ? 1 : 0);

    for(int32_t col = bColId; col <= eColId; col ++)
        for(int32_t row = bRowId; row <= eRowId; row ++)
            if(col >= 0 && row >= 0 && col < _tile_map->colCount() && row < _tile_map->rowCount())
            {
                const sp<RenderObject>& tile = _tile_map->getTile(row, col);
                if(tile)
                {
                    uint32_t rigidBodyId = row * _tile_map->colCount() + col;
                    candidates.insert(rigidBodyId);
                    contacts.insert(rigidBodyId);

                    bool notInContacts = _contacts.find(rigidBodyId) == _contacts.end();
                    if(notInContacts)
                    {
                        updateRigidBodyTile(rigidBodyId, tileWidth, tileHeight, _tile_map->colCount());
                        const V normal(col == bColId ? 1.0f : (col == eColId ? -1.0f : 0.0f),
                                       row == bRowId ? 1.0f : (row == eRowId ? -1.0f : 0.0f));
                        callback()->onBeginContact(_rigid_body_tile, CollisionManifold(normal));
                    }
                }
            }

    for(uint32_t id : candidates)
        if(contacts.find(id) == contacts.end())
        {
            updateRigidBodyTile(id, tileWidth, tileHeight, _tile_map->colCount());
            _rigid_body_tile->setId(id);
            callback()->onEndContact(_rigid_body_tile);
        }
    _contacts = contacts;
}

TiledCollider::RigidBodyTile::RigidBodyTile(uint32_t width, uint32_t height)
    : RigidBody(0, Collider::BODY_TYPE_STATIC, sp<Vec2::Impl>::make(V2()), sp<Size>::make(static_cast<float>(width), static_cast<float>(height)), nullptr)
{
    _position = position();
}

void TiledCollider::RigidBodyTile::setId(uint32_t id)
{
    _stub->_id = id;
}

void TiledCollider::RigidBodyTile::setPosition(float x, float y)
{
    _position->set(V2(x, y));
}

void TiledCollider::RigidBodyTile::dispose()
{
}

}
