#ifndef ARK_RENDERER_UTIL_ELEMENT_UTIL_H_
#define ARK_RENDERER_UTIL_ELEMENT_UTIL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

namespace ark {

class ElementUtil {
public:
    static bytearray makeUnitCubeVertices();

    static sp<Model> makeUnitQuadModel();
    static sp<Model> makeUnitNinePatchModel();

    static element_index_t hash(const sp<IndexArray>& indices);

    static bool isScissorEnabled(const Rect& scissor);
};

}

#endif
