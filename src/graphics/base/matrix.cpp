#include "graphics/base/matrix.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/util/strings.h"

#include "graphics/base/v3.h"

namespace ark {

Matrix::Matrix()
{
    DCHECK(sizeof(_value) == sizeof(glm::mat4), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(glm::mat4));
    setIdentity();
}

Matrix::Matrix(const Matrix& other)
{
    DCHECK(sizeof(_value) == sizeof(glm::mat4), "Matrix size unmatch: %d != %d", sizeof(_value), sizeof(glm::mat4));
    memcpy(_value, other._value, sizeof(_value));
}

Matrix Matrix::operator *(const Matrix& right) const
{
    return Matrix(matrix<glm::mat4>() * right.matrix<glm::mat4>());
}

Matrix& Matrix::operator =(const Matrix& other)
{
    matrix<glm::mat4>() = other.matrix<glm::mat4>();
    return *this;
}

void Matrix::setIdentity()
{
    matrix<glm::mat4>() = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
}

void Matrix::translate(float x, float y, float z)
{
    matrix<glm::mat4>() = glm::translate(matrix<glm::mat4>(), glm::vec3(x, y, z));
}

void Matrix::rotate(float degree, float x, float y, float z)
{
    matrix<glm::mat4>() = glm::rotate(matrix<glm::mat4>(), degree, glm::vec3(x, y, z));
}

void Matrix::scale(float x, float y, float z)
{
    matrix<glm::mat4>() = glm::scale(matrix<glm::mat4>(), glm::vec3(x, y, z));
}

float* Matrix::value()
{
    return _value;
}

const float* Matrix::value() const
{
    return _value;
}

void Matrix::map(const float x, const float y, const float z, float& transformedX, float& transformedY, float& transformedZ) const
{
    const glm::vec4 transformed = matrix<glm::mat4>() * glm::vec4(x, y, z, 1.0f);
    transformedX = transformed.x;
    transformedY = transformed.y;
    transformedZ = transformed.z;
}

template<> ARK_API String Strings::toString<Matrix>(const Matrix& value)
{
    return Strings::join<float>(value.value(), 0, 16);
}

}

