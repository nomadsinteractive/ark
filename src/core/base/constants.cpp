#include "core/base/constants.h"

#include "core/util/uploader_type.h"

#include "graphics/base/boundaries.h"

#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/impl/vertices/vertices_nine_patch_quads.h"
#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/impl/vertices/vertices_quad.h"

namespace ark {

namespace {

sp<Model> makeUnitQuadModel(const sp<Boundaries>& content)
{
    return sp<Model>::make(UploaderType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1})), sp<VerticesQuad>::make(), content);
}

sp<Model> makeUnitNinePatchTriangleStripsModel(const sp<Boundaries>& content)
{
    return sp<Model>::make(UploaderType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 4, 1, 5, 2, 6, 3, 7, 7, 4, 4, 8, 5, 9, 6, 10, 7, 11, 11, 8, 8, 12, 9, 13, 10, 14, 11, 15})), sp<VerticesNinePatchTriangleStrips>::make(), content);
}

sp<Model> makeUnitNinePatchQuadsModel(const sp<Boundaries>& content)
{
    return sp<Model>::make(UploaderType::makeElementIndexInput(std::initializer_list<element_index_t>({0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21, 24, 26, 25, 26, 27, 25, 28, 30, 29, 30, 31, 29, 32, 34, 33, 34, 35, 33})), sp<VerticesNinePatchQuads>::make(), content);
}

}

Constants::Constants()
    : BOOLEAN_TRUE(sp<Boolean>::make<Boolean::Const>(true)), BOOLEAN_FALSE(sp<Boolean>::make<Boolean::Const>(false)), BOUNDARIES_UNIT(sp<Boundaries>::make(V3(-0.5f), V3(0.5f))), MODEL_UNIT_QUAD(makeUnitQuadModel(BOUNDARIES_UNIT)),
      MODEL_UNIT_NINE_PATCH_TRIANGLE_STRIPS(makeUnitNinePatchTriangleStripsModel(BOUNDARIES_UNIT)), MODEL_UNIT_NINE_PATCH_QUADS(makeUnitNinePatchQuadsModel(BOUNDARIES_UNIT)),
      NUMERIC_ZERO(sp<Numeric>::make<Numeric::Const>(0)), NUMERIC_ONE(sp<Numeric>::make<Numeric::Const>(1.0f))
{
}

}
