#include "app/base/level_library.h"

namespace ark {

LevelLibrary::LevelLibrary(int32_t id, String name, sp<Vec3> size, sp<Shape> shape)
    : _id(id), _name(std::move(name)), _size(std::move(size)), _shape(std::move(shape))
{
}

int32_t LevelLibrary::id() const
{
    return _id;
}

const String& LevelLibrary::name() const
{
    return _name;
}

const sp<Vec3>& LevelLibrary::size() const
{
    return _size;
}

const sp<Shape>& LevelLibrary::shape() const
{
    return _shape;
}

void LevelLibrary::setShape(sp<Shape> shape)
{
    _shape = std::move(shape);
}

}
