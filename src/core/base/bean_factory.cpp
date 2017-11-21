#include "core/base/bean_factory.h"

namespace ark {

BeanFactory::BeanFactory(std::nullptr_t)
{
}

BeanFactory::BeanFactory()
    : _references(sp<Scope>::make()), _factories(sp<List<Factory>>::make()) {
}

BeanFactory::~BeanFactory()
{
}

void BeanFactory::add(const BeanFactory::Factory& factory, bool front)
{
    if(front)
        _factories->push_front(factory);
    else
        _factories->push_back(factory);
}

void BeanFactory::addPackage(const String& name, const BeanFactory& package)
{
    _packages[name] = sp<BeanFactory>::make(package);
}

void BeanFactory::extend(const BeanFactory& other)
{
    for(const Factory& i : other._factories->items())
        _factories->push_back(i);
}

const sp<Scope>& BeanFactory::references() const
{
    return _references;
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

}
