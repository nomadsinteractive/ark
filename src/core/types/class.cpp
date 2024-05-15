#include "core/types/class.h"

#include "core/base/class_manager.h"
#include "core/types/box.h"

namespace ark {

namespace {

class DefaultClassImpl : public IClass {
public:
    virtual Box cast(const Box& box, TypeId id) override {
        if(box.typeId() == id)
            return box;
        return Box();
    }
};

}

Class::Class()
    : _id(0), _name(nullptr), _delegate(nullptr)
{
}

Class::Class(TypeId id)
    : _id(id), _name("<Unknown>")
{
    static DefaultClassImpl impl;
    _delegate = &impl;
    _implements.insert(id);
}

Class::Class(TypeId id, const char* name, IClass* delegate)
    : _id(id), _name(name), _delegate(delegate)
{
    _implements.insert(id);
}

TypeId Class::id() const
{
    return _id;
}

const char* Class::name() const
{
    return _name;
}

const std::set<TypeId>& Class::implements() const
{
    return _implements;
}

bool Class::isInstance(TypeId id) const
{
    return _implements.find(id) != _implements.end();
}

Box Class::cast(const Box& box, TypeId id)
{
    return _delegate->cast(box, id);
}

void Class::setImplementation(std::set<TypeId>&& implementation)
{
    _implements = implementation;
}

Class* Class::getClass(TypeId id)
{
    return ClassManager::instance().obtain(id);
}

Class* Class::putClass(TypeId id, const char* name, IClass* impl)
{
    return ClassManager::instance().addClass(id, name, impl);
}

}
