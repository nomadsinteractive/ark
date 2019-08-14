#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_CALLABLE_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_CALLABLE_DUCK_TYPE_H_

#include "core/inf/duck.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PyCallableDuckType : public Duck<Runnable>, public Duck<EventListener>, public Duck<CharacterMaker>, public Duck<CharacterMapper>, public Duck<RendererMaker>,
        public Implements<PyCallableDuckType, Duck<Runnable>, Duck<EventListener>, Duck<CharacterMaker>, Duck<CharacterMapper>, Duck<RendererMaker>> {
public:
    PyCallableDuckType(PyInstance inst);

    virtual void to(sp<Runnable>& inst) override;
    virtual void to(sp<EventListener>& inst) override;
    virtual void to(sp<CharacterMaker>& inst) override;
    virtual void to(sp<CharacterMapper>& inst) override;
    virtual void to(sp<RendererMaker>& inst) override;

private:
    PyInstance _instance;
};

}
}
}
#endif
