#ifndef ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_NUMERIC_DUCK_TYPE_H_
#define ARK_PLUGIN_PYTHON_IMPL_DUCK_PY_NUMERIC_DUCK_TYPE_H_

#include "core/forwarding.h"
#include "core/inf/duck.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"

namespace ark {
namespace plugin {
namespace python {

class PyNumericDuckType : public Duck<Integer>, public Duck<Numeric>, public Implements<PyNumericDuckType, Duck<Integer>, Duck<Numeric>> {
public:
    PyNumericDuckType(const sp<PyInstance>& inst);

    virtual void to(sp<Integer>& inst) override;
    virtual void to(sp<Numeric>& inst) override;

private:
    sp<PyInstance> _instance;

};

}
}
}

#endif
