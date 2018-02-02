#include "renderer/base/atlas.h"

#include "core/base/bean_factory.h"
#include "core/dom/document.h"
#include "core/inf/dictionary.h"
#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/rect.h"
#include "graphics/base/size.h"

#include "renderer/base/gl_texture.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

Atlas::Atlas(const sp<GLTexture>& texture, bool allowDefaultItem)
    : _texture(texture), _half_pixel_x(static_cast<uint16_t>(32768 / texture->width())), _half_pixel_y(static_cast<uint16_t>(32768 / texture->height())),
      _atlas(new ByIndex<Item>()), _allow_default_item(allowDefaultItem)
{
}

const sp<GLTexture>& Atlas::texture() const
{
    return _texture;
}

uint32_t Atlas::width() const
{
    return _texture->width();
}

uint32_t Atlas::height() const
{
    return _texture->height();
}

uint16_t Atlas::halfPixelX() const
{
    return _half_pixel_x;
}

uint16_t Atlas::halfPixelY() const
{
    return _half_pixel_y;
}

bool Atlas::has(uint32_t c) const
{
    return _atlas->has(c);
}

void Atlas::add(uint32_t id, uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX, float pivotY)
{
    _atlas->add(id, makeItem(left, top, right, bottom, pivotX, pivotY));
}

const Atlas::Item& Atlas::at(uint32_t id) const
{
    return _allow_default_item ? (has(id) ? _atlas->at(id) : _default_item) : _atlas->at(id);
}

void Atlas::clear()
{
    _atlas.reset(new ByIndex<Item>());
}

uint16_t Atlas::unnormalize(uint32_t x, uint32_t s)
{
    return static_cast<uint16_t>(x * 65536 / s);
}

Atlas::Item Atlas::makeItem(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom, float pivotX, float pivotY) const
{
    uint16_t l = unnormalize(left, _texture->width());
    uint16_t t = unnormalize(top, _texture->height());
    uint16_t r = unnormalize(right, _texture->width());
    uint16_t b = unnormalize(bottom, _texture->height());
    float width = static_cast<float>(right - left);
    float height = static_cast<float>(bottom - top);
    return g_isOriginBottom ? Item(l + _half_pixel_x, b - _half_pixel_y, r - _half_pixel_x, t + _half_pixel_y, width, height, pivotX, pivotY)
                            : Item(l + _half_pixel_x, t + _half_pixel_y, r - _half_pixel_x, b - _half_pixel_y, width, height, pivotX, pivotY);
}

Atlas::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _factory(factory), _manifest(manifest), _atlas(factory.getBuilder<Atlas>(manifest, Constants::Attributes::ATLAS, false)),
      _texture(factory.getBuilder<GLTexture>(manifest, Constants::Attributes::SRC)),
      _resource_loader_context(resourceLoaderContext)
{
}

sp<Atlas> Atlas::BUILDER::build(const sp<Scope>& args)
{
    if(_atlas)
        return _atlas->build(args);

    const sp<GLTexture> texture = _texture->build(args);
    DCHECK(texture, "Build atlas or texture from \"%s\" failed", Documents::toString(_manifest).c_str());
    const sp<Atlas> atlas = sp<Atlas>::make(_texture->build(args));
    for(const document& i : _manifest->children())
    {
        if(i->name() == "import")
        {
            const sp<Atlas::Importer> importer = _factory.ensure<Atlas::Importer>(i, Constants::Attributes::TYPE, args);
            importer->import(atlas, _resource_loader_context, i);
        }
        else
        {
            uint32_t type = Documents::getAttribute<uint32_t>(i, Constants::Attributes::TYPE, 0);
            if(type == 0)
            {
                const String character = Documents::getAttribute(i, "character");
                if(character)
                    type = character.at(0);
            }
            const Rect r = Rect::parse(i);
            atlas->add(type, static_cast<uint32_t>(r.left()), static_cast<uint32_t>(r.top()), static_cast<uint32_t>(r.right()), static_cast<uint32_t>(r.bottom()),
                       Documents::getAttribute<float>(i, "pivot-x", 0), Documents::getAttribute<float>(i, "pivot-y", 0));
        }
    }
    return atlas;
}

Atlas::Item::Item()
    : _left(0), _top(0), _right(0), _bottom(0), _size(Null::ptr<Size>()), _pivot_x(0), _pivot_y(0)
{
}

Atlas::Item::Item(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, float width, float height, float pivotX, float pivotY)
    : _left(left), _top(top), _right(right), _bottom(bottom), _size(sp<Size>::make(width, height)), _pivot_x(pivotX), _pivot_y(pivotY)
{
}

const sp<Size>& Atlas::Item::size() const
{
    return _size;
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
    return _size->width();
}

float Atlas::Item::height() const
{
    return _size->height();
}

float Atlas::Item::pivotX() const
{
    return _pivot_x;
}

float Atlas::Item::pivotY() const
{
    return _pivot_y;
}

}
