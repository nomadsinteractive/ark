#include "python/extension/py_garbage_collector.h"

#include "core/util/log.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

PyGarbageCollector::PyGarbageCollector(const sp<PyInstance>& garbage)
    : _garbage(garbage)
{
}

int PyGarbageCollector::traverse(visitproc visit, void* arg)
{
    if (_garbage)
    {
        int vret = visit(_garbage->instance(), arg);
        if(vret != 0)
            return vret;
    }
    return 0;
}

int PyGarbageCollector::clear()
{
    Py_XDECREF(_garbage->release());
    _garbage = nullptr;
    return 0;
}

}
}
}
