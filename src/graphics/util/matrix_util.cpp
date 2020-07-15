#include "graphics/util/matrix_util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "core/util/math.h"

#include "graphics/base/mat.h"
#include "graphics/base/v4.h"

namespace ark {

M2 MatrixUtil::mul(const M2& lvalue, const M2& rvalue)
{
    return M2(lvalue.mat<glm::mat2>() * rvalue.mat<glm::mat2>());
}

V2 MatrixUtil::mul(const M2& lvalue, const V2& rvalue)
{
    const glm::vec2 r = lvalue.mat<glm::mat2>() * glm::vec2(rvalue.x(), rvalue.y());
    return V2(r.x, r.y);
}

M2 MatrixUtil::rotate(const M2& lvalue, float radian)
{
    float cos = Math::cos(radian);
    float sin = Math::sin(radian);
    float rot[4] = {cos, sin, -sin, cos};
    return lvalue * M2(rot);
}

M2 MatrixUtil::scale(const M2& lvalue, const V2& rvalue)
{
    float scale[4] = {rvalue.x(), 0, 0, rvalue.y()};
    return lvalue * M2(scale);
}

M3 MatrixUtil::mul(const M3& lvalue, const M3& rvalue)
{
    return M3(lvalue.mat<glm::mat3>() * rvalue.mat<glm::mat3>());
}

V3 MatrixUtil::mul(const M3& lvalue, const V3& rvalue)
{
    const glm::vec3 r = lvalue.mat<glm::mat3>() * glm::vec3(rvalue.x(), rvalue.y(), rvalue.z());
    return V3(r.x, r.y, r.z);
}

V2 MatrixUtil::mul(const M3& lvalue, const V2& rvalue)
{
    const V3 r = mul(lvalue, V3(rvalue, 1.0f));
    DCHECK(r.z() != 0, "Division by zero");
    return V2(r.x() / r.z(), r.y() / r.z());
}

M3 MatrixUtil::rotate(const M3& lvalue, float radian)
{
    return M3(glm::rotate(lvalue.mat<glm::mat3>(), radian));
}

M3 MatrixUtil::scale(const M3& lvalue, const V2& rvalue)
{
    return M3(glm::scale(lvalue.mat<glm::mat3>(), *reinterpret_cast<const glm::vec2*>(&rvalue)));
}

M3 MatrixUtil::translate(const M3& lvalue, const V2& rvalue)
{
    return M3(glm::translate(lvalue.mat<glm::mat3>(), *reinterpret_cast<const glm::vec2*>(&rvalue)));
}

M4 MatrixUtil::mul(const M4& lvalue, const M4& rvalue)
{
    return M4(lvalue.mat<glm::mat4>() * rvalue.mat<glm::mat4>());
}

V4 MatrixUtil::mul(const M4& lvalue, const V4& rvalue)
{
    const glm::vec4 r = lvalue.mat<glm::mat4>() * glm::vec4(rvalue.x(), rvalue.y(), rvalue.z(), rvalue.w());
    return V4(r.x, r.y, r.z, r.w);
}

V3 MatrixUtil::mul(const M4& lvalue, const V3& rvalue)
{
    const glm::vec4 r = lvalue.mat<glm::mat4>() * glm::vec4(rvalue.x(), rvalue.y(), rvalue.z(), 1.0f);
    DCHECK(r.w != 0, "Division by zero");
    return V3(r.x / r.w, r.y / r.w, r.z / r.w);
}

M4 MatrixUtil::rotate(const M4& lvalue, const V3& direction, float radian)
{
    return M4(glm::rotate(lvalue.mat<glm::mat4>(), radian, *reinterpret_cast<const glm::vec3*>(&direction)));
}

M4 MatrixUtil::scale(const M4& lvalue, const V3& rvalue)
{
    return M4(glm::scale(lvalue.mat<glm::mat4>(), *reinterpret_cast<const glm::vec3*>(&rvalue)));
}

M4 MatrixUtil::translate(const M4& lvalue, const V3& rvalue)
{
    return M4(glm::translate(lvalue.mat<glm::mat4>(), *reinterpret_cast<const glm::vec3*>(&rvalue)));
}

V2 MatrixUtil::transform(const M3& matrix, const V2& pos)
{
    const V3 p = mul(matrix, V3(pos, 1.0f));
    return V2(p.x() / p.z(), p.y() / p.z());
}

V3 MatrixUtil::transform(const M4& matrix, const V3& pos)
{
    const V4 p = mul(matrix, V4(pos, 1.0f));
    return V3(p.x() / p.w(), p.y() / p.w(), p.z() / p.w());
}

M3 MatrixUtil::transpose(const M3& matrix)
{
    return glm::transpose(glm::make_mat3(reinterpret_cast<const float*>(&matrix)));
}

M4 MatrixUtil::transpose(const M4& matrix)
{
    return glm::transpose(glm::make_mat4(reinterpret_cast<const float*>(&matrix)));
}

M4 MatrixUtil::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::ortho(left, right, bottom, top, near, far));
}

M4 MatrixUtil::lookAt(const V3& position, const V3& target, const V3& up)
{
    return M4(glm::lookAt(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z())));
}

M4 MatrixUtil::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return M4(glm::frustum(left, right, bottom, top, near, far));
}

M4 MatrixUtil::perspective(float fov, float aspect, float near, float far)
{
    return M4(glm::perspective(fov, aspect, near, far));
}

}
