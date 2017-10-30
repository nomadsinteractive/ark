#include "core/types/box.h"

#include "core/types/class.h"
#include "core/types/interfaces.h"

namespace ark {

Box::Box() noexcept
{
}

Box::Box(void* instance, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor&& destructor)
    : _instance(std::make_shared<Stub>(instance, typeId, interfaces, std::move(destructor)))
{
}

Box::Box(const Box& other)
    : _instance(other._instance)
{
}

Box::Box(Box&& other)
    : _instance(std::move(other._instance))
{
}

TypeId Box::typeId() const
{
    return _instance ? _instance->typeId() : 0;
}

const Box& Box::operator =(const Box& other)
{
    _instance = other._instance;
    return *this;
}

const Box& Box::operator =(Box&& other)
{
    _instance = std::move(other._instance);
    return *this;
}

Box::operator bool() const
{
    return _instance && _instance->ptr() != nullptr;
}

Box::Stub::Stub(void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor&& destructor)
    : _ptr(ptr), _type_id(typeId), _interfaces(interfaces), _destructor(destructor)
{
}

Box::Stub::~Stub()
{
    if(_ptr)
        _destructor(_ptr);
}

void* Box::Stub::ptr() const
{
    return _ptr;
}

TypeId Box::Stub::typeId() const
{
    return _type_id;
}

const std::shared_ptr<Interfaces>& Box::Stub::interfaces() const
{
    return _interfaces;
}

}
