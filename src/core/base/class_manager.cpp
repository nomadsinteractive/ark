#include "core/base/class_manager.h"

#include "core/ark.h"

namespace ark {

Class* ClassManager::addClass(TypeId id, const char* name, IClass* impl)
{
    const std::scoped_lock<std::mutex> guard(_mutex);

    if(const auto iter = _classes.find(id); iter != _classes.end())
        return iter->second.get();

    Class* clazz = new Class(id, name, impl);
    _classes[id] = std::unique_ptr<Class>(clazz);
    return clazz;
}

Class* ClassManager::obtain(TypeId id)
{
    const std::scoped_lock<std::mutex> guard(_mutex);

    if(const auto iter = _classes.find(id); iter != _classes.end())
        return iter->second.get();

    Class* clazz = new Class(id);
    _classes[id] = std::unique_ptr<Class>(clazz);
    return clazz;
}

void ClassManager::updateHierarchy()
{
}

ClassManager& ClassManager::instance()
{
    return Ark::instance()._class_manager;
}

}
