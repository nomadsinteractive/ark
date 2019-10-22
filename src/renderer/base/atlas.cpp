#include "renderer/base/atlas.h"

#include "core/base/bean_factory.h"
#include "core/dom/dom_document.h"
#include "core/inf/dictionary.h"
#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"

#include "renderer/base/texture.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

Atlas::Atlas(const sp<Texture>& texture, bool allowDefaultItem)
    : _texture(texture), _items(new ByIndex<Item>()), _allow_default_item(allowDefaultItem)
{
}

const sp<Texture>& Atlas::texture() const
{
    return _texture;
}

const op<ByIndex<Atlas::Item>>& Atlas::items() const
{
    return _items;
}

uint32_t Atlas::width() const
{
    return static_cast<uint32_t>(_texture->width());
}

uint32_t Atlas::height() const
{
    return static_cast<uint32_t>(_texture->height());
}

bool Atlas::has(int32_t c) const
{
    return _items->has(c);
}

const V2& Atlas::getOriginalSize(int32_t c) const
{
    return at(c).size();
}

void Atlas::add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot)
{
    _items->add(id, makeItem(ux, uy, vx, vy, bounds, size, pivot));
}

const Atlas::Item& Atlas::at(int32_t id) const
{
    return _allow_default_item ? (has(id) ? _items->at(id) : _default_item) : _items->at(id);
}

void Atlas::getOriginalPosition(int32_t id, Rect& position) const
{
    const Atlas::Item& item = at(id);
    float nw = _texture->width() / 65536.0f;
    float nh = _texture->height() / 65536.0f;
    position = Rect(item.ux() * nw, item.vy() * nh, item.vx() * nw, item.uy() * nh);
}

void Atlas::clear()
{
    _items.reset(new ByIndex<Item>());
}

uint16_t Atlas::unnormalize(uint32_t x, uint32_t s)
{
    return static_cast<uint16_t>(std::min<uint32_t>(x * 65536 / s, std::numeric_limits<uint16_t>::max()));
}

Atlas::Item Atlas::makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const
{
    uint16_t l = unnormalize(ux, static_cast<uint32_t>(_texture->width()));
    uint16_t t = unnormalize(uy, static_cast<uint32_t>(_texture->height()));
    uint16_t r = unnormalize(vx, static_cast<uint32_t>(_texture->width()));
    uint16_t b = unnormalize(vy, static_cast<uint32_t>(_texture->height()));
    return Item(l, b, r, t, Rect(bounds.left() - pivot.x(), bounds.top() - pivot.y(), bounds.right() - pivot.x(), bounds.bottom() - pivot.y()), size);
}

Atlas::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _atlas(factory.getBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)),
      _texture(factory.ensureConcreteClassBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)),
      _resource_loader_context(resourceLoaderContext)
{
}

sp<Atlas> Atlas::BUILDER::build(const Scope& args)
{
    if(_atlas)
        return _atlas->build(args);

    const sp<Texture> texture = _texture->build(args);
    DCHECK(texture, "Build atlas or texture from \"%s\" failed", Documents::toString(_manifest).c_str());
    const sp<Atlas> atlas = sp<Atlas>::make(texture);
    for(const document& i : _manifest->children())
    {
        if(i->name() == "import")
        {
            const sp<Atlas::Importer> importer = _factory.ensure<Atlas::Importer>(i, args);
            importer->import(atlas, i);
        }
        else if(i->name() != Constants::Attributes::TEXTURE)
        {
            DCHECK(i->name() == "item", "No rule to import item \"%s\"", Documents::toString(i).c_str());
            int32_t type = Documents::getAttribute<int32_t>(i, Constants::Attributes::TYPE, 0);
            if(type == 0)
            {
                const String character = Documents::getAttribute(i, "character");
                if(character)
                    type = static_cast<int32_t>(character.at(0));
            }
            const Rect r = Rect::parse(i);
            float px = Documents::getAttribute<float>(i, "pivot-x", 0);
            float py = Documents::getAttribute<float>(i, "pivot-x", 0);
            atlas->add(type, static_cast<uint32_t>(r.left()), static_cast<uint32_t>(r.top()), static_cast<uint32_t>(r.right()), static_cast<uint32_t>(r.bottom()),
                       Rect(0, 0, 1.0f, 1.0f), V2(r.width(), r.height()), V2(px, py));
        }
    }
    return atlas;
}

Atlas::Item::Item()
    : _ux(0), _uy(0), _vx(0), _vy(0)
{
}

Atlas::Item::Item(uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy, const Rect& bounds, const V2& size)
    : _ux(ux), _uy(uy), _vx(vx), _vy(vy), _bounds(bounds), _size(size)
{
}

const Rect& Atlas::Item::bounds() const
{
    return _bounds;
}

const V2& Atlas::Item::size() const
{
    return _size;
}

uint16_t Atlas::Item::ux() const
{
    return _ux;
}

uint16_t Atlas::Item::uy() const
{
    return _uy;
}

uint16_t Atlas::Item::vx() const
{
    return _vx;
}

uint16_t Atlas::Item::vy() const
{
    return _vy;
}

}
