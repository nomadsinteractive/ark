#ifndef ARK_APP_UTIL_BODY_DEF_CUTE_C2_H_
#define ARK_APP_UTIL_BODY_DEF_CUTE_C2_H_

#include <vector>

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/util/shape_cute_c2.h"

namespace ark {

class BodyDefCuteC2 {
public:
    BodyDefCuteC2(int32_t shapeId, uint32_t width, uint32_t height, std::vector<sp<ShapeCuteC2>> shapes);
    BodyDefCuteC2(const document& manifest);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(BodyDefCuteC2);

    int32_t shapeId() const;

    const std::vector<sp<ShapeCuteC2>>& shapes() const;

private:
    int32_t _shape_id;

    uint32_t _width;
    uint32_t _height;

    std::vector<sp<ShapeCuteC2>> _shapes;
};

}

#endif
