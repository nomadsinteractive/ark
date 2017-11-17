#include "renderer/impl/gl_model/gl_model_nine_patch.h"

#include "core/impl/array/fixed_array.h"
#include "core/util/documents.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/filter.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/size.h"
#include "graphics/base/transform.h"
#include "graphics/base/v2.h"
#include "graphics/base/vec2.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"

namespace ark {

GLModelNinePatch::Item::Item(const Rect& bounds, const Rect& stretching, uint32_t textureWidth, uint32_t textureHeight)
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

GLModelNinePatch::Item::Item(const GLModelNinePatch::Item& other)
    : paddings(other.paddings)
{
    memcpy(_x, other._x, sizeof(_x));
    memcpy(_y, other._y, sizeof(_y));
}

GLModelNinePatch::Item::Item()
{
}

GLModelNinePatch::GLModelNinePatch(const sp<GLShader>& shader, const document& manifest, const sp<Atlas>& atlas)
    : _tex_coordinate_offset(shader->getAttribute("TexCoordinate").offset()), _stride(shader->stride())
{
    DCHECK(_tex_coordinate_offset % 2 == 0 && _tex_coordinate_offset < _stride, "Illegal TexCoordinate offset: %d, stride: %d", _tex_coordinate_offset, _stride);
    _tex_coordinate_offset /= 2;

    uint32_t textureWidth = atlas->texture()->width();
    uint32_t textureHeight = atlas->texture()->height();
    for(const document& node : manifest->children())
    {
        uint32_t type = Documents::getAttribute<uint32_t>(node, Constants::Attributes::TYPE, 0);
        const Rect patches = Documents::ensureAttribute<Rect>(node, Constants::Attributes::NINE_PATCH_PATCHES);
        bool hasBounds = atlas->has(type);
        if(hasBounds)
        {
            const Atlas::Item& item = atlas->at(type);
            const Rect r(item.left() * textureWidth / 65536.0f - 0.5f, (g_isOriginBottom ? item.bottom() : item.top()) * textureHeight / 65536.0f - 0.5f,
                         item.right() * textureWidth / 65536.0f + 0.5f, (g_isOriginBottom ? item.top() : item.bottom()) * textureHeight / 65536.0f + 0.5f);
            _nine_patch_items.make(type, r, patches, textureWidth, textureHeight);
        }
        else
        {
            const Rect r = Rect::parse(node);
            _nine_patch_items.make(type, r, patches, textureWidth, textureHeight);
        }
    }
}

array<uint8_t> GLModelNinePatch::getArrayBuffer(GLResourceManager& resourceManager, const LayerContext::Snapshot& renderContext, float x, float y) {
    const uint32_t size = renderContext._items.size();
    const uint32_t floatStride = _stride / 4;
    DCHECK(size > 0, "Empty RenderContext");
    const array<uint8_t> preallocated = resourceManager.getPreallocatedArray(size * 16 * _stride);
    GLfloat* buf = reinterpret_cast<GLfloat*>(preallocated->array());
    memset(buf, 0, preallocated->length());
    for(const RenderObject::Snapshot& renderObject : renderContext._items)
    {
        const Transform::Snapshot& transform = renderObject._transform;
        const sp<Filter>& filter = renderObject._filter;
        const V& position = renderObject._position;
        Rect paintRect(position.x(), position.y(), position.x() + renderObject._size.x(), position.y() + renderObject._size.y());
        if(!transform.disabled)
        {
            paintRect.translate(transform.translate.x() - transform.pivot.x(), transform.translate.y() - transform.pivot.y());
        }
        const Item& ninePatch = _nine_patch_items.at(renderObject._type);
        filter->setVaryings(buf, _stride, 16);
        fillPaintingRect(buf, paintRect, ninePatch, floatStride, x, y);
        buf += (_stride * 4);
    }
    return preallocated;
}

GLBuffer GLModelNinePatch::getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext)
{
    const uint32_t size = renderContext._items.size();
    return size ? glResourceManager.getGLIndexBuffer(GLResourceManager::BUFFER_NAME_NINE_PATCH, size * 30 - 2) : GLBuffer();
}

uint32_t GLModelNinePatch::mode() const
{
    return static_cast<uint32_t>(GL_TRIANGLE_STRIP);
}

void GLModelNinePatch::fillPaintingRect(float* buf, const Rect& paintRect, const Item& bounds, uint32_t floatStride, float x, float y) const
{
    const Rect& paddings = bounds.paddings;
    FixedArray<float, 4> xArray = {paintRect.left(), paintRect.left() + paddings.left(), paintRect.right() - paddings.right(), paintRect.right()};
    FixedArray<float, 4> yArray = {g_isOriginBottom ? paintRect.bottom() : paintRect.top(), g_isOriginBottom ? paintRect.bottom() - paddings.bottom() : paintRect.top() + paddings.top(),
                                   g_isOriginBottom ? paintRect.top() + paddings.top() : paintRect.bottom() - paddings.bottom(), g_isOriginBottom ? paintRect.top() : paintRect.bottom()};
    fillMesh(buf, xArray, yArray, floatStride, 0, x, y);
    fillTexCoord(buf, bounds, floatStride * 2, _tex_coordinate_offset);
}

void GLModelNinePatch::fillMesh(float* mesh, Array<float>& xArray, Array<float>& yArray, uint32_t stride, uint32_t offset, float x, float y) const
{
    uint32_t i, j;
    uint32_t xLen = xArray.length(), yLen = yArray.length();
    float* xData = xArray.array();
    float* yData = yArray.array();
    for(i = 0; i < yLen; i++) {
        for(j = 0; j < xLen; j++) {
            mesh[offset] = xData[j] + x;
            mesh[offset + 1] = yData[i] + y;
            mesh += stride;
        }
    }
}

void GLModelNinePatch::fillTexCoord(void* mesh, const GLModelNinePatch::Item& item, uint32_t shortStride, uint32_t offset) const
{
    uint16_t* ptr = reinterpret_cast<uint16_t*>(mesh);
    uint32_t i, j;
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            ptr[offset] = item._x[j];
            ptr[offset + 1] = item._y[i];
            ptr += shortStride;
        }
    }
}

}
