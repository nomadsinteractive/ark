#pragma once

#include "core/inf/duck.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/forwarding.h"
#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class [[deprecated]] PyCallableDuckType final : public Duck<Runnable>, public Duck<EventListener>, public Duck<GlyphMaker>, public Duck<RendererMaker>,
        public Implements<PyCallableDuckType, Duck<Runnable>, Duck<EventListener>, Duck<GlyphMaker>, Duck<RendererMaker>> {
public:
    PyCallableDuckType(PyInstance inst);

    void to(sp<Runnable>& inst) override;
    void to(sp<EventListener>& inst) override;
    void to(sp<GlyphMaker>& inst) override;
    void to(sp<RendererMaker>& inst) override;

private:
    PyInstance _instance;
};

}
