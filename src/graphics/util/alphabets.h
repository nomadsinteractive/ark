#ifndef ARK_GRAPHICS_UTIL_ALPHABETS_H_
#define ARK_GRAPHICS_UTIL_ALPHABETS_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class Alphabets {
public:
    struct Characters {
        Characters(const sp<List<sp<RenderObject>>>& renderObjects, const sp<Size>& size);
        Characters(const Characters& other);
        Characters();

        sp<List<sp<RenderObject>>> _render_objects;
        sp<Size> _size;
    };

    static Characters create(Alphabet& alphabet, const std::wstring& text, float textScale, float letterSpacing = 4.0f, float x = 0, float y = 0, float width = 0, float lineHeight = 0, float lineIndent = 0);
    static Characters create(const Atlas& atlas, const std::wstring& text, float letterSpacing = 4.0f, float x = 0, float y = 0, float width = 0, float lineHeight = 0, float lineIndent = 0);

private:
    Alphabets();
    static void place(float boundary, float letterSpacing, float lineIndent, const sp<Size>& size, float x, wchar_t c, float& flowx, float& flowy, float& fontHeight, float& lineOffset, const sp<List<sp<RenderObject>>>& renderObjects);
};

}

#endif
