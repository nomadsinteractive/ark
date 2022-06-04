#ifndef ARK_PLUGIN_PYTHON_IMPL_ADAPTER_RENDERER_MAKER_PYTHON_H_
#define ARK_PLUGIN_PYTHON_IMPL_ADAPTER_RENDERER_MAKER_PYTHON_H_

#include "core/inf/holder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/tile_maker.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class RendererMakerPython : public RendererMaker, public Holder, Implements<RendererMakerPython, RendererMaker, Holder> {
public:
    RendererMakerPython(PyInstance maker);
    RendererMakerPython(PyInstance maker, PyInstance recycler);

    virtual std::vector<sp<Renderer>> make(int32_t x, int32_t y) override;
    virtual void recycle(const sp<Renderer>& renderer) override;

    virtual void traverse(const Visitor& visitor) override;

private:
    PyInstance _maker;
    PyInstance _recycler;
};

}
}
}

#endif
