#ifndef ARK_PLUGIN_PYTHON_IMPL_TILE_MAKER_PYTHON_CALLABLE_TILE_MAKER_H_
#define ARK_PLUGIN_PYTHON_IMPL_TILE_MAKER_PYTHON_CALLABLE_TILE_MAKER_H_

#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/tile_maker.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableTileMaker : public TileMaker {
public:
    PythonCallableTileMaker(const sp<PyInstance>& callable);

    virtual sp<Renderer> makeTile(int32_t x, int32_t y) override;

private:
    sp<PyInstance> _callable;
    PyInstance _args;

};

}
}
}

#endif
