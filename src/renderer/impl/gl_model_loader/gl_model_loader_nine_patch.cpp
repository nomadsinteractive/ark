#include "renderer/impl/gl_model_loader/gl_model_loader_nine_patch.h"

#include "core/dom/document.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_model_buffer.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture.h"

namespace ark {

GLModelLoaderNinePatch::Item::Item(const Rect& bounds, const Rect& stretching, uint32_t textureWidth, uint32_t textureHeight)
    : paddings(stretching)
{
    _x[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left()), textureWidth);
    _x[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + stretching.left()), textureWidth);
    _x[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + stretching.right()), textureWidth);
    _x[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.right()), textureWidth);

    _y[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top()), textureHeight);
    _y[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + stretching.top()), textureHeight);
    _y[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + stretching.bottom()), textureHeight);
    _y[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.bottom()), textureHeight);

    paddings.setRight(bounds.width() - stretching.right());
    paddings.setBottom(bounds.height() - stretching.bottom());
}

GLModelLoaderNinePatch::GLModelLoaderNinePatch(const document& manifest, const sp<Atlas>& atlas)
    : GLModelLoader(GL_TRIANGLE_STRIP), _atlas(atlas)
{
    uint32_t textureWidth = atlas->texture()->width();
    uint32_t textureHeight = atlas->texture()->height();
    for(const document& node : manifest->children("render-object"))
    {
        uint32_t type = Documents::getAttribute<uint32_t>(node, Constants::Attributes::TYPE, 0);
        const Rect patches = Documents::ensureAttribute<Rect>(node, Constants::Attributes::NINE_PATCH_PATCHES);
        bool hasBounds = atlas->has(type);
        if(hasBounds)
        {
            const Atlas::Item& item = atlas->at(type);
            const Rect r(item.left() * textureWidth / 65536.0f - 0.5f, item.bottom() * textureHeight / 65536.0f - 0.5f,
                         item.right() * textureWidth / 65536.0f + 0.5f, item.top() * textureHeight / 65536.0f + 0.5f);
            _nine_patch_items.make(type, r, patches, textureWidth, textureHeight);
        }
        else
        {
            const Rect r = Rect::parse(node);
            _nine_patch_items.make(type, r, patches, textureWidth, textureHeight);
        }
    }
}

uint32_t GLModelLoaderNinePatch::estimateVertexCount(uint32_t renderObjectCount)
{
    return 16 * renderObjectCount;
}

void GLModelLoaderNinePatch::load(GLModelBuffer& buf, uint32_t type, const V& size)
{
    const Rect paintRect(0, 0, size.x(), size.y());
    const Item& ninePatch = _nine_patch_items.at(type);

    fillPaintingRect(buf, paintRect, ninePatch);
}

GLBuffer GLModelLoaderNinePatch::getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount)
{
    return renderObjectCount ? glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_NINE_PATCH, renderObjectCount * 30 - 2) : GLBuffer();
}

void GLModelLoaderNinePatch::fillPaintingRect(GLModelBuffer& buf, const Rect& paintRect, const Item& item) const
{
    const Rect& paddings = item.paddings;
    float xData[4] = {paintRect.left(), paintRect.left() + paddings.left(), paintRect.right() - paddings.right(), paintRect.right()};
    float yData[4] = {paintRect.bottom(), paintRect.bottom() - paddings.bottom(), paintRect.top() + paddings.top(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.setPosition(xData[j], yData[i], 0);
            buf.setTexCoordinate(item._x[j], item._y[i]);
            buf.nextVertex();
        }
    }
}

GLModelLoaderNinePatch::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _manifest(manifest), _atlas(factory.ensureBuilder<Atlas>(manifest))
{
}

sp<GLModelLoader> GLModelLoaderNinePatch::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelLoaderNinePatch>::make(_manifest, _atlas->build(args));
}

}
