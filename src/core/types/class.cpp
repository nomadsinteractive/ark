#include "core/types/class.h"

#include "core/base/class_manager.h"

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

Class::Class(const Class& other)
    : _id(other._id), _name(other._name), _implements(other._implements), _delegate(other._delegate)
{
}

Class::Class(Class&& other)
    : _id(other._id), _name(other._name), _implements(std::move(other._implements)), _delegate(other._delegate)
{
}

TypeId Class::id() const
{
    return _id;
}

const char* Class::name() const
{
    return _name;
}

const std::unordered_set<TypeId>& Class::implements() const
{
    return _implements;
}

bool Class::isInstance(TypeId id) const
{
    return _implements.find(id) != _implements.end();
}

const Class& Class::operator =(const Class& other)
{
    _id = other._id;
    _name = other._name;
    _implements = other._implements;
    _delegate = other._delegate;
    return *this;
}

const Class& Class::operator =(Class&& other)
{
    _id = other._id;
    _name = other._name;
    _implements = std::move(other._implements);
    _delegate = other._delegate;
    return *this;
}

Box Class::cast(const Box& box, TypeId id)
{
    return _delegate->cast(box, id);
}

void Class::setImplementation(std::unordered_set<TypeId>&& implementation)
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
