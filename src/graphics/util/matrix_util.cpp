#include "graphics/util/matrix_util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/quaternion.hpp>

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
    return {r.x, r.y};
}

M2 MatrixUtil::rotate(const M2& lvalue, float radian)
{
    const float cos = Math::cos(radian);
    const float sin = Math::sin(radian);
    const float rot[4] = {cos, sin, -sin, cos};
    return lvalue * M2(rot);
}

M2 MatrixUtil::scale(const M2& lvalue, const V2& rvalue)
{
    float scale[4] = {rvalue.x(), 0, 0, rvalue.y()};
    return lvalue * M2(scale);
}

M3 MatrixUtil::mul(const M3& lvalue, const M3& rvalue)
{
    return {lvalue.mat<glm::mat3>() * rvalue.mat<glm::mat3>()};
}

V3 MatrixUtil::mul(const M3& lvalue, const V3& rvalue)
{
    const glm::vec3 r = lvalue.mat<glm::mat3>() * glm::vec3(rvalue.x(), rvalue.y(), rvalue.z());
    return {r.x, r.y, r.z};
}

V2 MatrixUtil::mul(const M3& lvalue, const V2& rvalue)
{
    const V3 r = mul(lvalue, V3(rvalue, 1.0f));
    DCHECK(r.z() != 0, "Division by zero");
    return {r.x() / r.z(), r.y() / r.z()};
}

M3 MatrixUtil::rotate(const M3& lvalue, float radian)
{
    return {glm::rotate(lvalue.mat<glm::mat3>(), radian)};
}

M3 MatrixUtil::rotate(const M3& lvalue, float s, float c)
{
    M3 lt = lvalue.transpose();
    M3 result;
    const V3* lcols = reinterpret_cast<const V3*>(lt.value());
    V3* rcols = reinterpret_cast<V3*>(result.value());
    rcols[0] = lcols[0] * c + lcols[1] * s;
    rcols[1] = lcols[0] * -s + lcols[1] * c;
    rcols[2] = lcols[2];
    return result.transpose();
}

M3 MatrixUtil::scale(const M3& lvalue, const V2& rvalue)
{
    return {glm::scale(lvalue.mat<glm::mat3>(), *reinterpret_cast<const glm::vec2*>(&rvalue))};
}

M3 MatrixUtil::translate(const M3& lvalue, const V2& rvalue)
{
    return {glm::translate(lvalue.mat<glm::mat3>(), *reinterpret_cast<const glm::vec2*>(&rvalue))};
}

M4 MatrixUtil::mul(const M4& lvalue, const M4& rvalue)
{
    return {lvalue.mat<glm::mat4>() * rvalue.mat<glm::mat4>()};
}

V4 MatrixUtil::mul(const M4& lvalue, const V4& rvalue)
{
    const glm::vec4 r = lvalue.mat<glm::mat4>() * glm::vec4(rvalue.x(), rvalue.y(), rvalue.z(), rvalue.w());
    return {r.x, r.y, r.z, r.w};
}

V3 MatrixUtil::mul(const M4& lhs, const V3& rhs)
{
    const glm::vec4 r = lhs.mat<glm::mat4>() * glm::vec4(rhs.x(), rhs.y(), rhs.z(), 1.0f);
    CHECK(r.w != 0, "Division by zero");
    return {r.x / r.w, r.y / r.w, r.z / r.w};
}

M4 MatrixUtil::rotate(const M4& lvalue, const V3& direction, float radian)
{
    return {glm::rotate(lvalue.mat<glm::mat4>(), radian, *reinterpret_cast<const glm::vec3*>(&direction))};
}

M4 MatrixUtil::rotate(const M4& lvalue, const V4& quaternion)
{
    return {lvalue.mat<glm::mat4>() * glm::toMat4(glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z()))};
}

M4 MatrixUtil::scale(const M4& lvalue, const V3& rvalue)
{
    return {glm::scale(lvalue.mat<glm::mat4>(), *reinterpret_cast<const glm::vec3*>(&rvalue))};
}

M4 MatrixUtil::translate(const M4& lvalue, const V3& rvalue)
{
    return {glm::translate(lvalue.mat<glm::mat4>(), *reinterpret_cast<const glm::vec3*>(&rvalue))};
}

V2 MatrixUtil::transform(const M3& matrix, const V2& pos)
{
    const V3 p = mul(matrix, V3(pos, 1.0f));
    return {p.x() / p.z(), p.y() / p.z()};
}

V3 MatrixUtil::transform(const M4& matrix, const V3& pos)
{
    const V4 p = mul(matrix, V4(pos, 1.0f));
    return {p.x() / p.w(), p.y() / p.w(), p.z() / p.w()};
}

V3 MatrixUtil::transform(const M4& matrix, const V3& pos, const V3& org)
{
    return transform(matrix, pos - org) + org;
}

M3 MatrixUtil::transpose(const M3& matrix)
{
    return glm::transpose(glm::make_mat3(matrix.value()));
}

M4 MatrixUtil::transpose(const M4& matrix)
{
    return glm::transpose(glm::make_mat4(matrix.value()));
}

M4 MatrixUtil::inverse(const M4& matrix)
{
    return glm::inverse(glm::make_mat4(matrix.value()));
}

M4 MatrixUtil::ortho(float left, float right, float bottom, float top, float near, float far)
{
    return {glm::ortho(left, right, bottom, top, near, far)};
}

M4 MatrixUtil::lookAt(const V3& position, const V3& target, const V3& up)
{
    return {glm::lookAt(glm::vec3(position.x(), position.y(), position.z()), glm::vec3(target.x(), target.y(), target.z()), glm::vec3(up.x(), up.y(), up.z()))};
}

M4 MatrixUtil::frustum(float left, float right, float bottom, float top, float near, float far)
{
    return {glm::frustum(left, right, bottom, top, near, far)};
}

M4 MatrixUtil::perspective(float fov, float aspect, float near, float far)
{
    return {glm::perspective(fov, aspect, near, far)};
}

}
