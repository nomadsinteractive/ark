#include "graphics/util/alphabets.h"

#include "core/collection/list.h"

#include "graphics/base/frame.h"
#include "graphics/base/size.h"
#include "graphics/base/render_object.h"
#include "graphics/base/vec2.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/alphabet.h"
#include "graphics/inf/layer.h"

#include "renderer/base/atlas.h"

namespace ark {

extern bool g_isOriginBottom;

void Alphabets::place(float boundary, float letterSpacing, float lineIndent, const sp<Size>& size, float x, wchar_t c, float& flowx, float& flowy, float& fontHeight, float& lineOffset, const sp<List<sp<RenderObject>>>& renderObjects)
{
    if(fontHeight == 0)
    {
        fontHeight = size->height();
        if(lineOffset == 0)
            lineOffset = g_isOriginBottom ? -fontHeight : fontHeight;
    }
    else
        flowx += letterSpacing;
    if(x != boundary)
    {
        if(flowx + size->width() > boundary)
        {
            flowy += lineOffset;
            flowx = x + lineIndent;
        }
    }
    const sp<Vec2> position = sp<Vec2>::make(flowx, flowy);
    renderObjects->push_back(sp<RenderObject>::make(c, position, size));
    flowx += size->width();
}

Alphabets::Characters Alphabets::create(Alphabet& alphabet, const std::wstring& text, float textScale, float letterSpacing, float x, float y, float width, float lineHeight, float lineIndent)
{
    float flowx = x, flowy = y, boundary = x + width;
    float lineOffset = g_isOriginBottom ? -lineHeight : lineHeight;
    float fontHeight = 0;
    const sp<List<sp<RenderObject>>> renderObjects = sp<List<sp<RenderObject>>>::make();
    for(auto iter = text.begin(); iter != text.end(); ++iter)
    {
        wchar_t c = *iter;
        uint32_t width, height;
        alphabet.load(c, width, height, false, false);
        const sp<Size> size = sp<Size>::make(static_cast<float>(textScale ? width * textScale : width), static_cast<float>(textScale ? height * textScale : height));
        place(boundary, letterSpacing, lineIndent, size, x, c, flowx, flowy, fontHeight, lineOffset, renderObjects);
    }
    return Characters(renderObjects, sp<Size>::make(flowx - x, flowy - y + (lineHeight == 0 ? fontHeight : lineHeight)));
}

Alphabets::Characters Alphabets::create(const Atlas& atlas, const std::wstring& text, float letterSpacing, float x, float y, float width, float lineHeight, float lineIndent)
{
    float flowx = x, flowy = y, boundary = x + width;
    float lineOffset = g_isOriginBottom ? -lineHeight : lineHeight;
    float fontHeight = 0;
    const sp<List<sp<RenderObject>>> renderObjects = sp<List<sp<RenderObject>>>::make();
    for(auto iter = text.begin(); iter != text.end(); ++iter)
    {
        wchar_t c = *iter;
        const Atlas::Item& item = atlas.at(c);
        const sp<Size>& size = item.size();
        place(boundary, letterSpacing, lineIndent, size, x, c, flowx, flowy, fontHeight, lineOffset, renderObjects);
    }
    return Characters(renderObjects, sp<Size>::make(flowx - x, flowy - y + (lineHeight == 0 ? fontHeight : lineHeight)));
}

Alphabets::Characters::Characters(const sp<List<sp<RenderObject>>>& renderObjects, const sp<Size>& size)
    : _render_objects(renderObjects), _size(size)
{
}

Alphabets::Characters::Characters(const Characters& other)
    : _render_objects(other._render_objects), _size(other._size)
{
}

Alphabets::Characters::Characters()
{
}

}
