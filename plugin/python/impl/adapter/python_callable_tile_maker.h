#ifndef ARK_PLUGIN_PYTHON_IMPL_TILE_MAKER_PYTHON_CALLABLE_TILE_MAKER_H_
#define ARK_PLUGIN_PYTHON_IMPL_TILE_MAKER_PYTHON_CALLABLE_TILE_MAKER_H_

#include "core/inf/holder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/tile_maker.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableTileMaker : public RendererMaker, public Holder, Implements<PythonCallableTileMaker, RendererMaker, Holder> {
public:
    PythonCallableTileMaker(PyInstance callable);

    virtual sp<Renderer> make(int32_t x, int32_t y) override;

    virtual void traverse(const Visitor& visitor) override;

private:
    PyInstance _callable;
    PyInstance _args;

};

}
}
}

#endif
