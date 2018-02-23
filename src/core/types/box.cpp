#include "core/types/box.h"

#include "core/types/class.h"
#include "core/types/interfaces.h"

namespace ark {

Box::Box(void* instance, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor)
    : _instance(std::make_shared<Stub>(instance, typeId, interfaces, destructor))
{
}

TypeId Box::typeId() const
{
    return _instance ? _instance->typeId() : 0;
}

Box::operator bool() const
{
    return _instance && _instance->ptr() != nullptr;
}

Box::Stub::Stub(void* ptr, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor)
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
