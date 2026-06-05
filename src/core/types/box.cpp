#include "core/types/box.h"

#include "core/types/class.h"
#include "core/util/math.h"

namespace ark {

Box::Box(const TypeId typeId, const Class* clazz, const void* sharedPtr, const void* instancePtr, Destructor destructor) noexcept
    : _type_id(typeId), _class(clazz), _stub_type(StubType::PTR),
      _stub(std::make_shared<std::any>(PtrStub(sharedPtr, instancePtr, std::move(destructor))))
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

bool Box::isEnum() const
{
    return _stub_type == StubType::ENUM;
}

bool Box::isFunction() const
{
    return _stub_type == StubType::FUNCTION;
}

int32_t Box::toEnumValue() const
{
    if(!_stub)
        return 0;

    ASSERT(_stub_type == StubType::ENUM);
    return std::any_cast<int32_t>(*_stub);
}

Box Box::cast(const TypeId typeId) const
{
    return _class->cast(*this, typeId);
}

uintptr_t Box::id() const
{
    if(!_stub)
        return 0;

    if(_stub_type == StubType::PTR)
        return reinterpret_cast<uintptr_t>(std::any_cast<const PtrStub&>(*_stub)._instance_ptr);

    int32_t hashvalue = _type_id;
    if(_stub_type == StubType::TRIVIAL)
    {
        const TrivialCopyableStorage& storage = std::any_cast<const TrivialCopyableStorage&>(*_stub);
        for(size_t i = 0; i < storage.size(); ++i)
            Math::hashCombine(hashvalue, storage[i]);
        return hashvalue;
    }

    Math::hashCombine(hashvalue, toEnumValue());
    return hashvalue;
}

Box::operator bool() const
{
    if(_stub)
    {
        if(_stub_type == StubType::PTR)
            return std::any_cast<const PtrStub&>(*_stub)._instance_ptr != nullptr;
        return true;
    }
    return false;
}

}
