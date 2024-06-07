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

uintptr_t Box::id() const
{
    return _stub ? reinterpret_cast<uintptr_t>(_stub.get()) : 0;
}

Box::operator bool() const
{
    if(_stub)
    {
        if(const PtrStub* ptrStub = std::get_if<PtrStub>(_stub.get()))
            return !ptrStub || ptrStub->instance_ptr != nullptr;
        return _ensure_enum_stub()->_value != 0;
    }
    return false;
}

}
