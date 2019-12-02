#ifndef ARK_GRAPHICS_UTIL_MATRIX_UTIL_H_
#define ARK_GRAPHICS_UTIL_MATRIX_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API MatrixUtil final {
public:

    static M3 mul(const M3& lvalue, const M3& rvalue);
    static V3 mul(const M3& lvalue, const V3& rvalue);

    static M4 mul(const M4& lvalue, const M4& rvalue);
    static V4 mul(const M4& lvalue, const V4& rvalue);

    static M4 rotate(const M4& lvalue, const V3& direction, float radian);
    static M4 scale(const M4& lvalue, const V3& rvalue);
    static M4 translate(const M4& lvalue, const V3& rvalue);

    static M4 ortho(float left, float right, float bottom, float top, float near, float far);
    static M4 lookAt(const V3& position, const V3& target, const V3& up);
    static M4 frustum(float left, float right, float bottom, float top, float near, float far);
    static M4 perspective(float fov, float aspect, float near, float far);

};

}

#endif
