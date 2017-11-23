#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_CALLABLE_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_CALLABLE_DUCK_TYPE_H_

#include "core/inf/duck.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyCallableDuckType : public Duck<Runnable>, public Duck<EventListener>, public Duck<TileMaker>, public Implements<PyCallableDuckType, Duck<Runnable>, Duck<EventListener>, Duck<TileMaker>> {
public:
    PyCallableDuckType(const sp<PyInstance>& inst);

    virtual void to(sp<Runnable>& inst) override;
    virtual void to(sp<EventListener>& inst) override;
    virtual void to(sp<TileMaker>& inst) override;

private:
    sp<PyInstance> _instance;
};

}
}
}
#endif
