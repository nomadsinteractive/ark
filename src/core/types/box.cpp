#include "core/types/box.h"

#include "core/types/class.h"

namespace ark {

Box::Box(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept
    : _stub(std::make_shared<_StubVariant>(PtrStub(typeId, clazz, sharedPtr, instancePtr, destructor)))
{
}

TypeId Box::typeId() const
{
    return _stub ? std::get<PtrStub>(*_stub).type_id : 0;
}

const void* Box::ptr() const
{
    return _stub ? std::get<PtrStub>(*_stub).instance_ptr : nullptr;
}

Box::operator bool() const
{
    return _stub && std::get<PtrStub>(*_stub).instance_ptr != nullptr;
}

}
