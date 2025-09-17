#include "core/base/bean_factory.h"

namespace ark {

BeanFactory::BeanFactory(sp<Dictionary<document>> documentRefs)
    : _stub(sp<Stub>::make(std::move(documentRefs))) {
}

void BeanFactory::add(BeanFactory::Factory factory, const bool front)
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

BeanFactory::Factory::operator bool() const
{
    return !_workers.table().empty();
}

BeanFactory::Stub::Stub(sp<Dictionary<document>> documentRefs)
    : _references(sp<Scope>::make()), _document_refs(std::move(documentRefs))
{
}

}
