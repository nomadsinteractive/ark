#include "extension/py_ark_boolean_type.h"

#include "core/impl/boolean/boolean_wrapper.h"

#include "python/extension/python_interpreter.h"

#include "generated/py_ark_method_def.h"

using namespace ark;

namespace ark {
namespace plugin {
namespace python {

static PyNumberMethods PyArkBoolean_tp_as_number = {
    0,                                                      /* binaryfunc nb_add;         */ /* __add__ */
    0,                                                      /* binaryfunc nb_subtract;    */ /* __sub__ */
    0,                                                      /* binaryfunc nb_multiply;    */ /* __mul__ */
    0,                                                      /* binaryfunc nb_remainder;   */ /* __mod__ */
    0,                                                      /* binaryfunc nb_divmod;      */ /* __divmod__ */
    0,                                                      /* ternaryfunc nb_power;      */ /* __pow__ */
    0,                                                      /* unaryfunc nb_negative;     */ /* __neg__ */
    0,                                                      /* unaryfunc nb_positive;     */ /* __pos__ */
    0,                                                      /* unaryfunc nb_absolute;     */ /* __abs__ */
    (inquiry) (PyArkBooleanType::__bool__),                 /* inquiry nb_bool;               */ /* __bool__ */
    0,                                                      /* unaryfunc nb_invert;       */ /* __invert__ */
    0,                                                      /* binaryfunc nb_lshift;      */ /* __lshift__ */
    0,                                                      /* binaryfunc nb_rshift;      */ /* __rshift__ */
    0,                                                      /* binaryfunc nb_and;         */ /* __and__ */
    0,                                                      /* binaryfunc nb_xor;         */ /* __xor__ */
    0,                                                      /* binaryfunc nb_or;          */ /* __or__ */
    0,                                                      /* unaryfunc nb_int;          */ /* __int__ */
    0,                                                      /* void *nb_reserved;         */
    0,                                                      /* unaryfunc nb_float;        */ /* __float__ */
    0,                                                      /* binaryfunc nb_inplace_add  */
    0,                                                      /* binaryfunc nb_inplace_subtract                */
    0,                                                      /* binaryfunc nb_inplace_multiply                */
    0,                                                      /* binaryfunc nb_inplace_remainder;              */
    0,                                                      /* ternaryfunc nb_inplace_power;                 */
    0,                                                      /* binaryfunc nb_inplace_lshift;                 */
    0,                                                      /* binaryfunc nb_inplace_rshift;                 */
    0,                                                      /* binaryfunc nb_inplace_and;                    */
    0,                                                      /* binaryfunc nb_inplace_xor;                    */
    0,                                                      /* binaryfunc nb_inplace_or;  */
    0,                                                      /* binaryfunc nb_floor_divide;                   */
    0,                                                      /* binaryfunc nb_true_divide;                    */ /* __div__ */
    0,                                                      /* binaryfunc nb_inplace_floor_divide;           */
    0,                                                      /* binaryfunc nb_inplace_true_divide             */ /* __idiv__ */
};

PyArkBooleanType::PyArkBooleanType(const String& name, const String& doc, long flags)
    : PyArkType(name, doc, flags)
{
    PyTypeObject* pyTypeObject = getPyTypeObject();
    pyTypeObject->tp_methods = PyArkBoolean_methods;
    pyTypeObject->tp_as_number = &PyArkBoolean_tp_as_number;
    pyTypeObject->tp_init = (initproc) __init__;
}

PyObject* PyArkBooleanType::val(Instance* self, PyObject* args)
{
    const sp<Boolean>& var = self->unpack<Boolean>();
    NOT_NULL(var);
    return PyBool_FromLong(var->val() ? 1 : 0);
}

PyObject* PyArkBooleanType::set(Instance* self, PyObject* args)
{
    PyObject* delegate;
    if(!PyArg_ParseTuple(args, "O", &delegate))
    {
        PyErr_SetString(PyExc_TypeError, "Argument mismatch");
        Py_RETURN_NAN;
    }
    const sp<Boolean>& var = self->unpack<Boolean>();
    if(var.is<Boolean::Impl>())
    {
        var.cast<Boolean::Impl>()->set(PyObject_IsTrue(delegate) != 0);
        Py_RETURN_NAN;
    }
    if(var.is<BooleanWrapper>())
    {
        const sp<BooleanWrapper> wrapped = var.as<BooleanWrapper>();
        if(PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(delegate)))
            wrapped->setDelegate(PythonInterpreter::instance()->toSharedPtr<Boolean>(delegate));
        else
            wrapped->set(PyObject_IsTrue(delegate) != 0);
        Py_RETURN_NAN;
    }
    PyErr_SetString(PyExc_TypeError, "Immutable variable being set");
    Py_RETURN_NAN;
}

int PyArkBooleanType::__bool__(Instance* self)
{
    const sp<Boolean>& var = self->unpack<Boolean>();
    NOT_NULL(var);
    return var->val() ? 1 : 0;
}

int PyArkBooleanType::__init__(Instance* self, PyObject* args, PyObject* /*kwargs*/)
{
    PyObject* delegate = Py_None;
    if(!PyArg_ParseTuple(args, "|O", &delegate))
    {
        PyErr_SetString(PyExc_TypeError, "Argument mismatch");
        return -1;
    }
    const sp<BooleanWrapper> wrapped = sp<BooleanWrapper>::make(false);
    if(PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(delegate)))
        wrapped->setDelegate(PythonInterpreter::instance()->toSharedPtr<Boolean>(delegate));
    else
        wrapped->set(PyObject_IsTrue(delegate) != 0);
    self->box = new Box(wrapped.cast<Boolean>().pack());
    return 0;
}

}
}
}
