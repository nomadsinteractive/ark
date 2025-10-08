#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API MatrixUtil final {
public:

    static M2 mul(const M2& lvalue, const M2& rvalue);
    static V2 mul(const M2& lvalue, const V2& rvalue);

    static M2 rotate(const M2& lvalue, float radian);
    static M2 scale(const M2& lvalue, const V2& rvalue);

    static M3 mul(const M3& lvalue, const M3& rvalue);
    static V3 mul(const M3& lvalue, const V3& rvalue);
    static V2 mul(const M3& lvalue, const V2& rvalue);

    static M3 rotate(const M3& lvalue, float radian);
    static M3 rotate(const M3& lvalue, float s, float c);
    static M3 scale(const M3& lvalue, const V2& rvalue);
    static M3 translate(const M3& lvalue, const V2& rvalue);

    static M4 mul(const M4& lvalue, const M4& rvalue);
    static V4 mul(const M4& lvalue, const V4& rvalue);
    static V3 mul(const M4& lhs, const V3& rhs);

    static M4 rotate(const M4& lvalue, const V3& direction, float radian);
    static M4 rotate(const M4& lvalue, const V4& quaternion);
    static M4 scale(const M4& lvalue, const V3& rvalue);
    static M4 translate(const M4& lvalue, const V3& rvalue);

    static V2 transform(const M3& matrix, const V2& pos);
    static V3 transform(const M4& matrix, const V3& pos);
    static V3 transform(const M4& matrix, const V3& pos, const V3& org);

    static M3 transpose(const M3& matrix);
    static M4 transpose(const M4& matrix);

    static M4 inverse(const M4& matrix);
};

}
