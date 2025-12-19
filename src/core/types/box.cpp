#include "core/types/box.h"

#include "core/types/class.h"
#include "core/util/math.h"

namespace ark {

Box::Box(const TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept
    : _type_id(typeId), _class(clazz), _stub(std::make_shared<_StubVariant>(PtrStub(sharedPtr, instancePtr, std::move(destructor))))
{
}

TypeId Box::typeId() const
{
    return _type_id;
}

const Class* Box::getClass() const
{
    return _class;
}

uintptr_t Box::id() const
{
    if(!_stub)
        return 0;

    if(PtrStub* stub = std::get_if<PtrStub>(_stub.get()))
        return reinterpret_cast<uintptr_t>(stub->instance_ptr);

    int32_t hashvalue = _ensure_enum_stub()->_value;
    Math::hashCombine(hashvalue, _type_id);
    return hashvalue;
}

Box::operator bool() const
{
    if(_stub)
    {
        if(const PtrStub* ptrStub = std::get_if<PtrStub>(_stub.get()))
            return ptrStub->instance_ptr != nullptr;
        return true;
    }
    return false;
}

}
