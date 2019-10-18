#ifndef ARK_RENDERER_UTIL_VERTEX_UTIL_H_
#define ARK_RENDERER_UTIL_VERTEX_UTIL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

namespace ark {

class VertexUtil {
public:
    static bytearray makeUnitCubeVertices();

    static bool isScissorEnabled(const Rect& scissor);
};

}

#endif
