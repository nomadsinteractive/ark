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
    : _texture(texture), _width(static_cast<float>(_texture->width())), _height(static_cast<float>(_texture->height())), _allow_default_item(allowDefaultItem)
{
}

void Atlas::load(BeanFactory& factory, const document& manifest, const Scope& args)
{
    for(const document& i : manifest->children())
    {
        if(i->name() == "import")
        {
            const sp<Atlas::Importer> importer = factory.ensure<Atlas::Importer>(i, args);
            importer->import(*this, factory, i);
        }
        else if(i->name() != Constants::Attributes::TEXTURE)
        {
            DCHECK(i->name() == "item", "No rule to import item \"%s\"", Documents::toString(i).c_str());
            int32_t type = Documents::getAttribute<int32_t>(i, Constants::Attributes::TYPE, 0);
            float px = Documents::getAttribute<float>(i, "pivot-x", 0);
            float py = Documents::getAttribute<float>(i, "pivot-x", 0);
            if(has(type))
            {
                const Item& item = at(type);
                _items[type] = Item(item.ux(), item.uy(), item.vx(), item.vy(), Rect(-px, -py, 1.0f - px, 1.0f - py), item.size());
            }
            else
            {
                const Rect r = Rect::parse(i);
                add(type, static_cast<uint32_t>(r.left()), static_cast<uint32_t>(r.top()), static_cast<uint32_t>(r.right()), static_cast<uint32_t>(r.bottom()),
                    Rect(0, 0, 1.0f, 1.0f), V2(r.width(), r.height()), V2(px, py));
            }
        }
    }
}

const sp<Texture>& Atlas::texture() const
{
    return _texture;
}

const std::unordered_map<int32_t, Atlas::Item>& Atlas::items() const
{
    return _items;
}

uint32_t Atlas::width() const
{
    return static_cast<uint32_t>(_width);
}

uint32_t Atlas::height() const
{
    return static_cast<uint32_t>(_height);
}

bool Atlas::has(int32_t c) const
{
    return _items.find(c) != _items.end();
}

const V2& Atlas::getOriginalSize(int32_t c) const
{
    return at(c).size();
}

Rect Atlas::getItemUV(int32_t c) const
{
    return at(c).uv();
}

ByType& Atlas::attachments()
{
    return _attachments;
}

void Atlas::add(int32_t id, uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot)
{
    _items[id] = makeItem(ux, uy, vx, vy, bounds, size, pivot);
}

const Atlas::Item& Atlas::at(int32_t id) const
{
    DCHECK((_allow_default_item || id == 0) || has(id), "Item(%d) does not exist", id);
    return _allow_default_item || id == 0 ? (has(id) ? _items.at(id) : _default_item) : _items.at(id);
}

Rect Atlas::getOriginalPosition(int32_t id) const
{
    const Atlas::Item& item = at(id);
    float nw = _width / 65536.0f;
    float nh = _height / 65536.0f;
    return Rect(item.ux() * nw, item.vy() * nh, item.vx() * nw, item.uy() * nh);
}

void Atlas::clear()
{
    _items.clear();
}

uint16_t Atlas::unnormalize(uint32_t x, uint32_t s)
{
    return static_cast<uint16_t>(std::min<uint32_t>(x * 65536 / s, std::numeric_limits<uint16_t>::max()));
}

Atlas::Item Atlas::makeItem(uint32_t ux, uint32_t uy, uint32_t vx, uint32_t vy, const Rect& bounds, const V2& size, const V2& pivot) const
{
    uint16_t l = unnormalize(ux, static_cast<uint32_t>(_width));
    uint16_t t = unnormalize(uy, static_cast<uint32_t>(_height));
    uint16_t r = unnormalize(vx, static_cast<uint32_t>(_width));
    uint16_t b = unnormalize(vy, static_cast<uint32_t>(_height));
    return Item(l, b, r, t, Rect(bounds.left() - pivot.x(), bounds.top() - pivot.y(), bounds.right() - pivot.x(), bounds.bottom() - pivot.y()), size);
}

Atlas::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _texture(factory.ensureConcreteClassBuilder<Texture>(manifest, Constants::Attributes::TEXTURE))
{
}

sp<Atlas> Atlas::BUILDER::build(const Scope& args)
{
    const sp<Texture> texture = _texture->build(args);
    DCHECK(texture, "Build texture from \"%s\" failed", Documents::toString(_manifest).c_str());
    const sp<Atlas> atlas = sp<Atlas>::make(texture);
    atlas->load(_factory, _manifest, args);
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

Rect Atlas::Item::uv() const
{
    return Rect(_ux / 65536.0f, _uy / 65536.0f, _vx / 65536.0f, _vy / 65536.0f);
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
