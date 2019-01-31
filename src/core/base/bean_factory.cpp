#include "core/base/bean_factory.h"

namespace ark {

BeanFactory::BeanFactory(std::nullptr_t)
{
}

BeanFactory::BeanFactory()
    : _stub(sp<Stub>::make()) {
}

BeanFactory::~BeanFactory()
{
}

void BeanFactory::add(const BeanFactory::Factory& factory, bool front)
{
    if(front)
        _stub->_factories.push_front(factory);
    else
        _stub->_factories.push_back(factory);
}

void BeanFactory::addPackage(const String& name, const BeanFactory& package)
{
    _stub->_packages[name] = sp<BeanFactory>::make(package);
}

void BeanFactory::extend(const BeanFactory& other)
{
    for(const Factory& i : other._stub->_factories.items())
        _stub->_factories.push_back(i);
}

const sp<BeanFactory>& BeanFactory::getPackage(const String& name) const
{
    DCHECK(name, "Empty package name");
    auto iter = _stub->_packages.find(name);
    return iter == _stub->_packages.end() ? sp<BeanFactory>::null() : iter->second;
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
