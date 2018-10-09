#include "core/types/box.h"

#include "core/types/class.h"
#include "core/types/interfaces.h"

namespace ark {

Box::Box(void* instance, TypeId typeId, const std::shared_ptr<Interfaces>& interfaces, Destructor destructor)
    : _stub(std::make_shared<Stub>(instance, typeId, interfaces, destructor))
{
}

TypeId Box::typeId() const
{
    return _stub ? _stub->typeId() : 0;
}

Box Box::toConcrete() const
{
    NOT_NULL(_stub);
    return _stub->_interfaces->as(*this, _stub->_interfaces->typeId());
}

void* Box::ptr() const
{
    return _stub ? _stub->ptr() : nullptr;
}

const std::shared_ptr<Interfaces>& Box::interfaces() const
{
    NOT_NULL(_stub);
    return _stub->_interfaces;
}

Box::operator bool() const
{
    return _stub && _stub->ptr() != nullptr;
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
