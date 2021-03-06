#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_META_TYPE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_META_TYPE_H_

#include "core/ark.h"
#include "core/epi/disposed.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"

#include "python/extension/python_interpreter.h"
#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

template<typename T> class PyArkMetaType {
public:
    static PyObject* expire(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Disposed>()) {
            const sp<Disposed> m = ptr.template as<Disposed>();
            if(m)
                m->dispose();
        }
        Py_RETURN_NONE;
    }

    static PyObject* isExpired(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Disposed>()) {
            const sp<Disposed> m = ptr.template as<Disposed>();
            if(m && m->val())
                Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }

    static PyObject* size(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Block>()) {
            sp<Block> block = ptr.template as<Block>();
            const SafePtr<Size>& size = block->size();
            if(size)
                return PythonInterpreter::instance()->pyNewObject<Size>(size);
        }
        Py_RETURN_NONE;
    }

};

}
}
}

#endif
