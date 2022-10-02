#ifndef ARK_RENDERER_UTIL_RENDER_UTIL_H_
#define ARK_RENDERER_UTIL_RENDER_UTIL_H_

#include "core/forwarding.h"
#include "core/inf/array.h"

#include "graphics/forwarding.h"

#include "renderer/base/pipeline_input.h"
#include "renderer/base/texture.h"

namespace ark {

class RenderUtil {
public:
    static bytearray makeUnitCubeVertices(bool flipWindingOrder);
    static Attribute makePredefinedAttribute(const String& name, const String& type);

    static Model makeUnitNinePatchTriangleStripsModel();
    static Model makeUnitNinePatchQuadsModel();
    static Model makeUnitPointModel();
    static Model makeUnitQuadModel();

    static uint32_t hash(const element_index_t* buf, size_t len);

    static String outAttributeName(const String& name, PipelineInput::ShaderStage preStage);

    static bool isScissorEnabled(const Rect& scissor);

    static uint32_t getComponentSize(Texture::Format format);

};

}

#endif
