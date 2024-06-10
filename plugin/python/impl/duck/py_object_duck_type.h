#pragma once

#include "core/inf/duck.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class PyObjectDuckType : public Duck<String>, public Duck<CollisionCallback>, public Duck<RendererMaker>, public Duck<Integer>, public Duck<Numeric>,
        public Implements<PyObjectDuckType, Duck<String>, Duck<CollisionCallback>, Duck<RendererMaker>, Duck<Integer>, Duck<Numeric>> {
public:
    PyObjectDuckType(PyInstance inst);

    virtual void to(sp<String>& inst) override;
    virtual void to(sp<CollisionCallback>& inst) override;
    virtual void to(sp<RendererMaker>& inst) override;
    virtual void to(sp<Integer>& inst) override;
    virtual void to(sp<Numeric>& inst) override;

private:
    PyInstance _instance;

};

}
