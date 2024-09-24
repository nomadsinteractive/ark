#pragma once

#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/tile_maker.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class RendererMakerPython : public RendererMaker, Implements<RendererMakerPython, RendererMaker> {
public:
    RendererMakerPython(PyInstance maker);
    RendererMakerPython(PyInstance maker, PyInstance recycler);

    virtual std::vector<Box> make(float x, float y) override;
    virtual void recycle(const Box& renderer) override;

private:
    PyInstance _maker;
    PyInstance _recycler;
};

}
