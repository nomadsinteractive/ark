#include "core/types/box.h"

#include "core/types/class.h"

namespace ark {

Box::Box(TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept
    : _type_id(typeId), _stub(std::make_shared<_StubVariant>(PtrStub(clazz, sharedPtr, instancePtr, destructor)))
{
}

TypeId Box::typeId() const
{
    return _type_id;
}

const void* Box::ptr() const
{
    return _stub ? std::get<PtrStub>(*_stub).instance_ptr : nullptr;
}

Box::operator bool() const
{
    if(_stub)
    {
        const PtrStub* ptrStub = std::get_if<PtrStub>(_stub.get());
        return !ptrStub || ptrStub->instance_ptr != nullptr;
    }
    return false;
}

}
