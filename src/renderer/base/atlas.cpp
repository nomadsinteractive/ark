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
    : _texture(texture), _half_pixel_x(static_cast<uint16_t>(32768 / texture->width())), _half_pixel_y(static_cast<uint16_t>(32768 / texture->height())),
      _items(new ByIndex<Item>()), _allow_default_item(allowDefaultItem)
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

uint16_t Atlas::halfPixelX() const
{
    return _half_pixel_x;
}

uint16_t Atlas::halfPixelY() const
{
    return _half_pixel_y;
}

bool Atlas::has(int32_t c) const
{
    return _items->has(c);
}

const V2& Atlas::getOriginalSize(int32_t c) const
{
    return at(c).size();
}

void Atlas::add(int32_t id, uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX, float pivotY)
{
    _items->add(id, makeItem(left, top, right, bottom, pivotX, pivotY));
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
    position = Rect((item.left() - _half_pixel_x) * nw, (item.bottom() - _half_pixel_y) * nh, (item.right() + _half_pixel_x) * nw, (item.top() + _half_pixel_y) * nh);
}

void Atlas::clear()
{
    _items.reset(new ByIndex<Item>());
}

uint16_t Atlas::unnormalize(uint32_t x, uint32_t s)
{
    return static_cast<uint16_t>(x * 65536 / s);
}

Atlas::Item Atlas::makeItem(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX, float pivotY) const
{
    uint16_t l = unnormalize(left, static_cast<uint32_t>(_texture->width()));
    uint16_t t = unnormalize(top, static_cast<uint32_t>(_texture->height()));
    uint16_t r = unnormalize(right, static_cast<uint32_t>(_texture->width()));
    uint16_t b = unnormalize(bottom, static_cast<uint32_t>(_texture->height()));
    float width = static_cast<float>(right - left);
    float height = static_cast<float>(bottom - top);
    return Item(l + _half_pixel_x, b - _half_pixel_y, r - _half_pixel_x, t + _half_pixel_y, width, height, pivotX, pivotY);
}

Atlas::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _atlas(factory.getBuilder<Atlas>(manifest, Constants::Attributes::ATLAS)),
      _texture(factory.ensureConcreteClassBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)),
      _resource_loader_context(resourceLoaderContext)
{
}

sp<Atlas> Atlas::BUILDER::build(const sp<Scope>& args)
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
            uint32_t type = Documents::getAttribute<uint32_t>(i, Constants::Attributes::TYPE, 0);
            if(type == 0)
            {
                const String character = Documents::getAttribute(i, "character");
                if(character)
                    type = static_cast<uint32_t>(character.at(0));
            }
            const Rect r = Rect::parse(i);
            atlas->add(type, static_cast<uint32_t>(r.left()), static_cast<uint32_t>(r.top()), static_cast<uint32_t>(r.right()), static_cast<uint32_t>(r.bottom()),
                       Documents::getAttribute<float>(i, "pivot-x", 0), Documents::getAttribute<float>(i, "pivot-y", 0));
        }
    }
    return atlas;
}

Atlas::Item::Item()
    : _left(0), _top(0), _right(0), _bottom(0)
{
}

Atlas::Item::Item(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, float width, float height, float pivotX, float pivotY)
    : _left(left), _top(top), _right(right), _bottom(bottom), _size(width, height), _pivot(pivotX, pivotY)
{
}

const V2& Atlas::Item::size() const
{
    return _size;
}

const V2& Atlas::Item::pivot() const
{
    return _pivot;
}

uint16_t Atlas::Item::left() const
{
    return _left;
}

uint16_t Atlas::Item::top() const
{
    return _top;
}

uint16_t Atlas::Item::right() const
{
    return _right;
}

uint16_t Atlas::Item::bottom() const
{
    return _bottom;
}

float Atlas::Item::width() const
{
    return _size.x();
}

float Atlas::Item::height() const
{
    return _size.y();
}

}
