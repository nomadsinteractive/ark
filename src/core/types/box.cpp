#include "core/types/box.h"

#include "core/types/class.h"
#include "core/types/interfaces.h"

namespace ark {

Box::Box(TypeId typeId, const void* sharedPtr, const void* instancePtr, std::shared_ptr<Interfaces> interfaces, Destructor destructor) noexcept
    : _stub(std::make_shared<_StubVariant>(PtrStub(typeId, sharedPtr, instancePtr, std::move(interfaces), destructor)))
{
}

TypeId Box::typeId() const
{
    return _stub ? std::get<PtrStub>(*_stub)._type_id : 0;
}

Box Box::toConcrete() const
{
    DASSERT(_stub);
    return std::get<PtrStub>(*_stub)._interfaces->as(*this, std::get<PtrStub>(*_stub)._interfaces->typeId());
}

const void* Box::ptr() const
{
    return _stub ? std::get<PtrStub>(*_stub)._instance_ptr : nullptr;
}

const std::shared_ptr<Interfaces>& Box::interfaces() const
{
    DASSERT(_stub);
    return std::get<PtrStub>(*_stub)._interfaces;
}

Box::operator bool() const
{
    return _stub && std::get<PtrStub>(*_stub)._instance_ptr != nullptr;
}

}
