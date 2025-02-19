#include "core/base/bean_factory.h"

namespace ark {

BeanFactory::BeanFactory(std::nullptr_t)
{
}

BeanFactory::BeanFactory()
    : _stub(sp<Stub>::make()) {
}

BeanFactory::BeanFactory(sp<Stub> stub)
    : _stub(std::move(stub))
{
}

void BeanFactory::add(BeanFactory::Factory factory, bool front)
{
    if(front)
        _stub->_factories.push_front(std::move(factory));
    else
        _stub->_factories.push_back(std::move(factory));
}

void BeanFactory::addPackage(const String& name, const BeanFactory& package)
{
    _stub->_packages[name] = sp<BeanFactory>::make(package);
}

void BeanFactory::extend(const BeanFactory& other)
{
    for(const Factory& i : other._stub->_factories)
        _stub->_factories.push_back(i);
}

sp<BeanFactory> BeanFactory::getPackage(const String& name) const
{
    DCHECK(name, "Empty package name");
    const auto iter = _stub->_packages.find(name);
    return iter != _stub->_packages.end() ? iter->second : nullptr;
}

const sp<Scope>& BeanFactory::references() const
{
    return _stub->_references;
}

BeanFactory::Factory::Factory()
{
}

BeanFactory::Factory::Factory(const WeakPtr<Scope>& references, const sp<Dictionary<document>>& documentById)
    : _references(references), _document_by_id(documentById)
{
}

BeanFactory::Factory::operator bool() const
{
    return static_cast<bool>(_document_by_id);
}

BeanFactory::Stub::Stub()
    : _references(sp<Scope>::make())
{
}

}
