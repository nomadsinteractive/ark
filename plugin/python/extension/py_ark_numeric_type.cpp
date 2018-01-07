#include "python/extension/py_ark_numeric_type.h"

#include "core/impl/numeric/add.h"
#include "core/impl/numeric/divide.h"
#include "core/impl/numeric/multiply.h"
#include "core/impl/numeric/negative.h"
#include "core/impl/numeric/scalar.h"
#include "core/impl/numeric/subtract.h"

#include "python/extension/python_interpreter.h"

#include "generated/py_ark_method_def.h"

using namespace ark;

namespace ark {
namespace plugin {
namespace python {

static PyNumberMethods PyArkNumeric_tp_as_number = {
    (binaryfunc) PyArkNumericType::__add__,       /* binaryfunc nb_add;                  */ /* __add__ */
    (binaryfunc) PyArkNumericType::__sub__,       /* binaryfunc nb_subtract;             */ /* __sub__ */
    (binaryfunc) PyArkNumericType::__mul__,       /* binaryfunc nb_multiply;             */ /* __mul__ */
    0,               /* binaryfunc nb_remainder;                                         */ /* __mod__ */
    0,               /* binaryfunc nb_divmod;                                            */ /* __divmod__ */
    0,               /* ternaryfunc nb_power;                                            */ /* __pow__ */
    (unaryfunc) PyArkNumericType::__neg__,        /* unaryfunc nb_negative;              */ /* __neg__ */
    0,               /* unaryfunc nb_positive;                                           */ /* __pos__ */
    0,               /* unaryfunc nb_absolute;                                           */ /* __abs__ */
    0,               /* inquiry nb_bool;                                                 */ /* __bool__ */
    0,               /* unaryfunc nb_invert;                                             */ /* __invert__ */
    0,               /* binaryfunc nb_lshift;                                            */ /* __lshift__ */
    0,               /* binaryfunc nb_rshift;                                            */ /* __rshift__ */
    0,               /* binaryfunc nb_and;                                               */ /* __and__ */
    0,               /* binaryfunc nb_xor;                                               */ /* __xor__ */
    0,               /* binaryfunc nb_or;                                                */ /* __or__ */
    0,               /* unaryfunc nb_int;                                                */ /* __int__ */
    0,               /* void *nb_reserved;                                               */
    (unaryfunc) PyArkNumericType::__float__,       /* unaryfunc nb_float;                */ /* __float__ */
    (binaryfunc) PyArkNumericType::__iadd__,       /* binaryfunc nb_inplace_add          */
    (binaryfunc) PyArkNumericType::__isub__,       /* binaryfunc nb_inplace_subtract     */
    (binaryfunc) PyArkNumericType::__imul__,       /* binaryfunc nb_inplace_multiply     */
    0,                                              /* binaryfunc nb_inplace_remainder;  */
    0,                                              /* ternaryfunc nb_inplace_power;     */
    0,                                              /* binaryfunc nb_inplace_lshift;     */
    0,                                              /* binaryfunc nb_inplace_rshift;     */
    0,                                              /* binaryfunc nb_inplace_and;        */
    0,                                              /* binaryfunc nb_inplace_xor;        */
    0,                                              /* binaryfunc nb_inplace_or;         */
    0,                                              /* binaryfunc nb_floor_divide;       */
    (binaryfunc) PyArkNumericType::__div__,        /* binaryfunc nb_true_divide;         */     /* __div__ */
    0,                                             /* binaryfunc nb_inplace_floor_divide;*/
    (binaryfunc) PyArkNumericType::__idiv__,       /* binaryfunc nb_inplace_true_divide  */    /* __idiv__ */

};

PyArkNumericType::PyArkNumericType(const String& name, const String& doc, long flags)
    : PyArkType(name, doc, flags)
{
    PyTypeObject* pyTypeObject = getPyTypeObject();
    pyTypeObject->tp_methods = PyArkNumeric_methods;
    pyTypeObject->tp_as_number = &PyArkNumeric_tp_as_number;
    pyTypeObject->tp_init = (initproc) __init__;
}

PyObject* PyArkNumericType::val(Instance* self, PyObject* /*args*/)
{
    const sp<Numeric>& var = self->unpack<Numeric>();
    return PyFloat_FromDouble((double) var->val());
}

PyObject* PyArkNumericType::set(Instance* self, PyObject* args)
{
    PyObject* val = Py_None;
    if(!PyArg_ParseTuple(args, "O", &val))
    {
        PyErr_SetString(PyExc_TypeError, "Numeric type needed, but wrong type of argument given");
        Py_RETURN_NAN;
    }
    const sp<Numeric>& inst = self->unpack<Numeric>();
    if(inst.is<Numeric::Impl>())
    {
        inst.cast<Numeric::Impl>()->set(static_cast<float>(PyFloat_AsDouble(val)));
        Py_RETURN_NAN;
    }
    if(inst.is<Scalar>())
    {
        const sp<Numeric> delegate = PythonInterpreter::instance()->asInterface<Numeric>(val);
        if(delegate)
            inst.cast<Scalar>()->setDelegate(delegate);
        else if(PyFloat_Check(val) || PyNumber_Check(val))
            inst.cast<Scalar>()->set(static_cast<float>(PyFloat_AsDouble(val)));
        else
            DFATAL("Cannot cast \"%s\" to Float, Number or ark::Numeric", Py_TYPE(val)->tp_name);
        Py_RETURN_NAN;
    }
    PyErr_SetString(PyExc_TypeError, "Immutable variable being set");
    Py_RETURN_NAN;
}

PyObject* PyArkNumericType::__add__(Instance* self, PyObject* args)
{
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric> ret = sp<Numeric>::adopt(new Add(unpacked, obj0));
    return PythonInterpreter::instance()->template fromSharedPtr<Numeric>(ret);
}

PyObject* PyArkNumericType::__sub__(Instance* self, PyObject* args)
{
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric> ret = sp<Numeric>::adopt(new Subtract(unpacked, obj0));
    return PythonInterpreter::instance()->template fromSharedPtr<Numeric>(ret);
}

PyObject* PyArkNumericType::__mul__(Instance* self, PyObject* args)
{
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric> ret = sp<Numeric>::adopt(new Multiply(unpacked, obj0));
    return PythonInterpreter::instance()->template fromSharedPtr<Numeric>(ret);
}

PyObject* PyArkNumericType::__neg__(Instance* self)
{
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    const sp<Numeric> ret = sp<Numeric>::adopt(new Negative(unpacked));
    return PythonInterpreter::instance()->template fromSharedPtr<Numeric>(ret);
}

PyObject* PyArkNumericType::__div__(Instance* self, PyObject* args)
{
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric> ret = sp<Numeric>::adopt(new Divide(unpacked, obj0));
    return PythonInterpreter::instance()->template fromSharedPtr<Numeric>(ret);
}

PyObject* PyArkNumericType::__truediv__(Instance* self, PyObject* args)
{
    return __div__(self, args);
}

PyObject* PyArkNumericType::__float__(Instance* self)
{
    return val(self, nullptr);
}

PyObject* PyArkNumericType::__iadd__(Instance* self, PyObject* args)
{
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    self->pack<Numeric>(sp<Numeric>::adopt(new Add(unpacked, obj0)));
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyArkNumericType::__isub__(Instance* self, PyObject* args)
{
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    self->pack<Numeric>(sp<Numeric>::adopt(new Subtract(unpacked, obj0)));
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyArkNumericType::__imul__(Instance* self, PyObject* args)
{
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    self->pack<Numeric>(sp<Numeric>::adopt(new Multiply(unpacked, obj0)));
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

PyObject* PyArkNumericType::__idiv__(Instance* self, PyObject* args)
{
    const sp<Numeric> obj0 = PythonInterpreter::instance()->toSharedPtr<Numeric>(args);
    const sp<Numeric>& unpacked = self->unpack<Numeric>();
    self->pack<Numeric>(sp<Numeric>::adopt(new Divide(unpacked, obj0)));
    Py_INCREF(self);
    return reinterpret_cast<PyObject*>(self);
}

int PyArkNumericType::__init__(Instance* self, PyObject* args, PyObject* /*kwargs*/)
{
    PyObject* delegate;
    if(!PyArg_ParseTuple(args, "O", &delegate))
    {
        PyErr_SetString(PyExc_TypeError, "Argument mismatch");
        return -1;
    }
    const sp<Scalar> wrapped = sp<Scalar>::make(0.0f);
    if(PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(delegate)))
        wrapped->setDelegate(PythonInterpreter::instance()->toSharedPtr<Numeric>(delegate));
    else
        wrapped->set(static_cast<float>(PyFloat_AsDouble(delegate)));
    self->box = new Box(wrapped.cast<Numeric>().pack());
    return 0;
}

}
}
}
