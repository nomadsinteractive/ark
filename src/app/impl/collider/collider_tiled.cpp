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

#include "app/base/application_context.h"
#include "app/base/rigid_body.h"
#include "app/base/tile_map.h"
#include "app/inf/collision_callback.h"

namespace ark {

namespace {

class DynamicPosition : public Vec2 {
public:
    DynamicPosition(const sp<TiledCollider::RigidBodyImpl>& rigidBody)
        : _body_stub(rigidBody->stub()), _bounds(0, 0, rigidBody->size()->width(), rigidBody->size()->height()) {
    }

    virtual V2 val() override {
        const V2 position = _body_stub->_position->val();
        _bounds.setCenter(position.x(), position.y());
        _body_stub->collision(_bounds);
        return position;
    }

private:
    sp<TiledCollider::RigidBodyImpl::Stub> _body_stub;
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

sp<RigidBody> TiledCollider::createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& /*rotate*/)
{
    DCHECK(type != Collider::BODY_TYPE_STATIC, "Cannot create static body in TiledCollider");
    NOT_NULL(position && size);

    const sp<RigidBodyImpl> rigidBody = sp<RigidBodyImpl>::make(++_rigid_body_base, type, position, size, _tile_map);
    rigidBody->setPosition(_resource_loader_context->synchronize<V>(sp<DynamicPosition>::make(rigidBody)));
    return rigidBody;
}

TiledCollider::RigidBodyImpl::RigidBodyImpl(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<TileMap>& tileMap)
    : RigidBody(id, type, position, size, Null::ptr<Rotate>()), _stub(tileMap ? sp<Stub>::make(tileMap, position) : sp<Stub>::null())
{
}

void TiledCollider::RigidBodyImpl::Stub::beginContact(const sp<RigidBody>& rigidBody)
{
}

void TiledCollider::RigidBodyImpl::Stub::endContact(const sp<RigidBody>& rigidBody)
{
}

void TiledCollider::RigidBodyImpl::Stub::updateRigidBodyStatic(uint32_t id, float tileWidth, float tileHeight, uint32_t colCount)
{
    uint32_t row = id / colCount;
    uint32_t col = id % colCount;
    _rigid_body_static->setId(id);
    _rigid_body_static->setPosition(col * tileWidth + tileWidth / 2.0f, row * tileHeight + tileHeight / 2.0f);
}

const sp<TiledCollider::RigidBodyImpl::Stub>& TiledCollider::RigidBodyImpl::stub() const
{
    return _stub;
}

void TiledCollider::RigidBodyImpl::dispose()
{
}

void TiledCollider::RigidBodyImpl::setPosition(const sp<Vec>& position)
{
    stub()->_position = position;
}

V2 TiledCollider::RigidBodyImpl::xy() const
{
    return _stub->_position->val();
}

TiledCollider::RigidBodyImpl::Stub::Stub(const sp<TileMap>& tileMap, const sp<Vec2>& position)
    : _position(position), _tile_map(tileMap), _rigid_body_static(sp<RigidBodyStatic>::make(tileMap->tileWidth(), tileMap->tileHeight()))
{
}

void TiledCollider::RigidBodyImpl::Stub::collision(const Rect& rect)
{
    std::set<uint32_t> candidates = _contacts;
    std::set<uint32_t> contacts;
    float tileWidth = static_cast<float>(_tile_map->tileWidth());
    float tileHeight = static_cast<float>(_tile_map->tileHeight());
    for(float x = Math::modFloor(rect.left(), tileWidth); x < rect.right(); x += tileWidth)
        for(float y = Math::modFloor(rect.top(), tileHeight); y < rect.bottom(); y += tileHeight)
        {
            if(_last_collision.ptin(x, y))
                continue;

            uint32_t col = static_cast<uint32_t>(x / tileWidth);
            uint32_t row = static_cast<uint32_t>(y / tileHeight);
            if(col < _tile_map->colCount() && row < _tile_map->rowCount())
            {
                const sp<RenderObject>& tile = _tile_map->getTile(row, col);
                if(tile)
                {
                    uint32_t rigidBodyId = row * _tile_map->colCount() + col;
                    candidates.insert(rigidBodyId);
                    contacts.insert(rigidBodyId);
                }
            }
        }

    for(uint32_t id : candidates)
        if(contacts.find(id) != contacts.end())
        {
            if(_contacts.find(id) == _contacts.end())
            {
                updateRigidBodyStatic(id, tileWidth, tileHeight, _tile_map->colCount());
                beginContact(_rigid_body_static);
            }
        }
        else
        {
            updateRigidBodyStatic(id, tileWidth, tileHeight, _tile_map->colCount());
            _rigid_body_static->setId(id);
            endContact(_rigid_body_static);
        }
    _contacts = contacts;
    _last_collision = rect;
}

TiledCollider::RigidBodyStatic::RigidBodyStatic(uint32_t width, uint32_t height)
    : RigidBody(0, Collider::BODY_TYPE_STATIC, sp<Vec2::Impl>::make(V2()), sp<Size>::make(static_cast<float>(width), static_cast<float>(height)), nullptr)
{
    _position = position();
}

void TiledCollider::RigidBodyStatic::setId(uint32_t id)
{
    _stub->_id = id;
}

void TiledCollider::RigidBodyStatic::setPosition(float x, float y)
{
    _position->set(V2(x, y));
}

void TiledCollider::RigidBodyStatic::dispose()
{
}

}
