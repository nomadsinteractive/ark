#ifndef ARK_RENDERER_UTIL_RENDER_UTIL_H_
#define ARK_RENDERER_UTIL_RENDER_UTIL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

#include "renderer/base/shader.h"

namespace ark {

class RenderUtil {
public:
    static bytearray makeUnitCubeVertices();

    static Model makeUnitNinePatchModel();
    static Model makeUnitPointModel();
    static Model makeUnitQuadModel();

    static element_index_t hash(element_index_t* buf, size_t len);

    static String outAttributeName(const String& name, Shader::Stage preStage);

    static bool isScissorEnabled(const Rect& scissor);
};

}

#endif
