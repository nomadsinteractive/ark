#include "app/traits/shape.h"

namespace ark {

Shape::Shape(int32_t id, sp<Size> size)
    : _id(id), _size(std::move(size))
{
}

int32_t Shape::id() const
{
    return _id;
}

void Shape::setId(int32_t id)
{
    _id = id;
}

const sp<Size>& Shape::size() const
{
    return _size;
}

void Shape::setSize(sp<Size> size)
{
    _size = std::move(size);
}

}
