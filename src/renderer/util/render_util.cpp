#include "renderer/util/render_util.h"

#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/base/v3.h"

#include "renderer/base/model.h"
#include "renderer/impl/vertices/vertices_nine_patch.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/impl/vertices/vertices_quad.h"
#include "renderer/inf/uploader.h"


namespace ark {

bytearray RenderUtil::makeUnitCubeVertices(bool flipWindingOrder)
{
    static float vertices[] = {
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f
    };

    if(flipWindingOrder)
    {
        sp<ByteArray::Fixed<sizeof(vertices)>> flipped = sp<ByteArray::Fixed<sizeof(vertices)>>::make();
        memcpy(flipped->buf(), vertices, flipped->size());
        V3* buf = reinterpret_cast<V3*>(flipped->buf());
        for(uint32_t i = 0; i < 6; ++i, buf += 4)
            std::swap(buf[1], buf[2]);
        return flipped;
    }

    return sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Model RenderUtil::makeUnitQuadModel()
{
    return Model(sp<Uploader::Array<element_index_t>>::make(sp<IndexArray::Fixed<6>>::make(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1}))), sp<VerticesQuad>::make(), Metrics{V3(1.0f), V3(1.0f), V3(0)});
}

Model RenderUtil::makeUnitNinePatchModel()
{
    return Model(sp<Uploader::Array<element_index_t>>::make(sp<IndexArray::Fixed<28>>::make(std::initializer_list<element_index_t>({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15}))), sp<VerticesNinePatch>::make());
}

Model RenderUtil::makeUnitPointModel()
{
    return Model(sp<Uploader::Array<element_index_t>>::make(sp<IndexArray::Fixed<1>>::make(std::initializer_list<element_index_t>({0}))), sp<VerticesPoint>::make(), Metrics{V3(1.0f), V3(1.0f), V3()});
}

element_index_t RenderUtil::hash(const element_index_t* buf, size_t len)
{
    element_index_t h = 0;
    for(size_t i = 0; i < len; ++i)
        h = h * 101 + buf[i];
    return h;
}

String RenderUtil::outAttributeName(const String& name, PipelineInput::ShaderStage preStage)
{
    DCHECK(preStage == PipelineInput::SHADER_STAGE_NONE || preStage == PipelineInput::SHADER_STAGE_VERTEX, "Only none and vertex stage's out attribute name supported");
    const char sPrefix[][8] = {"a_", "v_"};
    const String prefix = sPrefix[preStage + 1];
    return name.startsWith(prefix) ? name : prefix + Strings::capitalizeFirst(name);
}

bool RenderUtil::isScissorEnabled(const Rect& scissor)
{
    return scissor.right() > scissor.left() && scissor.bottom() > scissor.top();
}

}
