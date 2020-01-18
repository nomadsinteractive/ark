#include "renderer/util/element_util.h"

#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/base/v3.h"

#include "renderer/base/model.h"
#include "renderer/impl/vertices/vertices_nine_patch.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/impl/vertices/vertices_quad.h"


namespace ark {

bytearray ElementUtil::makeUnitCubeVertices()
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
    return sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(vertices), sizeof(vertices));
}

Model ElementUtil::makeUnitQuadModel()
{
    return Model(sp<IndexArray::Fixed<6>>::make(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1})), sp<VerticesQuad>::make(), Metrics{V3(1.0f), V3(1.0f), V3(0)});
}

Model ElementUtil::makeUnitNinePatchModel()
{
    return Model(sp<IndexArray::Fixed<28>>::make(std::initializer_list<element_index_t>({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15})), sp<VerticesNinePatch>::make());
}

Model ElementUtil::makeUnitPointModel()
{
    return Model(sp<IndexArray::Fixed<1>>::make(std::initializer_list<element_index_t>({0})), sp<VerticesPoint>::make(), Metrics{V3(1.0f), V3(1.0f), V3()});
}

element_index_t ElementUtil::hash(const sp<IndexArray>& indices)
{
    element_index_t* buf = indices->buf();
    size_t len = indices->length();
    element_index_t h = 0;
    for(size_t i = 0; i < len; ++i)
        h = h * 101 + buf[i];
    return h;
}

bool ElementUtil::isScissorEnabled(const Rect& scissor)
{
    return scissor.right() > scissor.left() && scissor.bottom() > scissor.top();
}

}
