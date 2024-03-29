#include "core/base/class_manager.h"

#include "core/ark.h"

namespace ark {

Class* ClassManager::addClass(TypeId id, const char* name, IClass* impl)
{
    const std::scoped_lock<std::mutex> guard(_mutex);
    _classes[id] = Class(id, name, impl);
    return &_classes[id];
}

Class* ClassManager::obtain(TypeId id)
{
    const std::scoped_lock<std::mutex> guard(_mutex);

    const auto iter = _classes.find(id);
    if(iter != _classes.end())
        return &iter->second;

    _classes[id] = Class(id);
    return &_classes[id];
}

void ClassManager::updateHierarchy()
{
}

ClassManager& ClassManager::instance()
{
    static ClassManager INSTANCE;
    return INSTANCE;
}

}
