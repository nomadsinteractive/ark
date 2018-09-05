#include "python/extension/py_garbage_collector.h"

#include "core/util/log.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

PyGarbageCollectorImpl::PyGarbageCollectorImpl(const sp<PyInstance>& garbage)
    : _garbage(garbage)
{
}

int PyGarbageCollectorImpl::traverse(visitproc visit, void* arg)
{
    Py_VISIT(_garbage->object());
    return 0;
}

int PyGarbageCollectorImpl::clear()
{
    _garbage->deref();
    _garbage = nullptr;
    return 0;
}

}
}
}
