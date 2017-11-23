#include "python/impl/duck/py_callable_duck_type.h"

#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"
#include "python/impl/adapter/python_callable_tile_maker.h"

namespace ark {
namespace plugin {
namespace python {

PyCallableDuckType::PyCallableDuckType(const sp<PyInstance>& inst)
    : _instance(inst)
{
}

void PyCallableDuckType::to(sp<Runnable>& inst)
{
    inst = sp<PythonCallableRunnable>::make(_instance);
}

void PyCallableDuckType::to(sp<EventListener>& inst)
{
    inst = sp<PythonCallableEventListener>::make(_instance);
}

void PyCallableDuckType::to(sp<TileMaker>& inst)
{
    inst = sp<PythonCallableTileMaker>::make(_instance);
}

}
}
}
