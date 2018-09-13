#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_META_TYPE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_META_TYPE_H_

#include "core/ark.h"
#include "core/epi/lifecycle.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"

#include "graphics/impl/renderer/renderer_delegate.h"

#include "python/extension/python_interpreter.h"
#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

template<typename T> class PyArkMetaType {
public:
    static PyObject* absorb(typename PyArkType::Instance* self, PyObject* args) {
        PyObject* arg1;
        if(PyArg_ParseTuple(args, "O", &arg1)) {
            sp<T>& ptr = self->unpack<T>();
            if(PythonInterpreter::instance()->isInstance<Lifecycle>(arg1)) {
                typename PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(arg1);
                ptr.absorb(instance->unpack<Lifecycle>());
            }
        }
        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }

    static PyObject* expire(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Lifecycle>()) {
            const sp<Lifecycle> m = ptr.template as<Lifecycle>();
            if(m)
                m->dispose();
        }
        Py_RETURN_NONE;
    }

    static PyObject* isExpired(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Lifecycle>()) {
            const sp<Lifecycle> m = ptr.template as<Lifecycle>();
            if(m && m->val())
                Py_RETURN_TRUE;
        }
        Py_RETURN_FALSE;
    }

    static PyObject* size(typename PyArkType::Instance* self, PyObject* /*args*/) {
        const sp<T>& ptr = self->unpack<T>();
        if(ptr.template is<Block>()) {
            sp<Block> block = ptr.template as<Block>();
            const sp<Size>& size = block->size();
            if(size)
                return PythonInterpreter::instance()->pyNewObject<Size>(size);
        }
        Py_RETURN_NONE;
    }

    static PyObject* addRenderer(typename PyArkType::Instance* self, PyObject* args) {
        PyObject* arg1;
        if(PyArg_ParseTuple(args, "O", &arg1)) {
            const sp<T>& ptr = self->unpack<T>();
            if(ptr.template is<Renderer::Group>()) {
                const sp<Renderer::Group> rendererGroup = ptr.template as<Renderer::Group>();
                rendererGroup->addRenderer(PythonInterpreter::instance()->asInterface<Renderer>(arg1));
            }
        }
        Py_RETURN_NONE;
    }

};

}
}
}

#endif
