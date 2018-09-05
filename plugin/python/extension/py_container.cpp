#include "python/extension/py_container.h"

#include "python/extension/py_garbage_collector.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

int PyContainer::traverse(visitproc visit, void* arg)
{
    if(_instances)
        Py_VISIT(_instances->object());

    for(auto iter = _garbage_collectors.begin(); iter != _garbage_collectors.end();)
    {
        const sp<PyGarbageCollector> gc = (*iter).lock();
        if(!gc)
        {
            iter = _garbage_collectors.erase(iter);
            if(iter == _garbage_collectors.end())
                break;
            continue;
        }
        ++iter;
        int r = gc->traverse(visit, arg);
        if(r)
            return r;
    }
    return 0;
}

int PyContainer::clear()
{
    if(_instances)
        _instances->deref();

    while(!_garbage_collectors.empty())
    {
        const sp<PyGarbageCollector> gc = _garbage_collectors.front().lock();
        _garbage_collectors.pop_front();
        if(gc)
        {
            int r = gc->clear();
            if(r)
                return r;
        }
    }
    return 0;
}

void PyContainer::addCollector(const WeakPtr<PyGarbageCollector>& collector)
{
    _garbage_collectors.push_back(collector);
}

void PyContainer::setPyInstance(const sp<PyInstance>& pyInstance)
{
    _instances = pyInstance;
}

}
}
}
