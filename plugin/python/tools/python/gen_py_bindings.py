#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
import re
import sys
from os import path

LOADER_TEMPLATE = '''template<typename T> static Box PyArk${classname}_${methodname}Function(PyArkType::Instance& inst, const String& id, const sp<Scope>& args) {
    const sp<T> bean = inst.unpack<${classname}>()->${methodname}<T>(id, args);
    return bean.pack();
}'''


TP_AS_NUMBER_TEMPLATE = [
    'static PyNumberMethods ${py_class_name}_tp_as_number = {',
    '    (binaryfunc) ${nb_add},        /* binaryfunc nb_add;                  */ /* __add__ */',
    '    (binaryfunc) ${nb_subtract}, /* binaryfunc nb_subtract; */ /* __sub__ */',
    '    (binaryfunc) ${nb_multiply}, /* binaryfunc nb_multiply; */ /* __mul__ */',
    '    (binaryfunc) ${nb_remainder}, /* binaryfunc nb_remainder; */ /* __mod__ */',
    '    0,               /* binaryfunc nb_divmod; */ /* __divmod__ */',
    '    0,               /* ternaryfunc nb_power; */ /* __pow__ */',
    '    (unaryfunc) ${nb_negative},        /* unaryfunc nb_negative; */ /* __neg__ */',
    '    0,               /* unaryfunc nb_positive; */ /* __pos__ */',
    '    0,               /* unaryfunc nb_absolute; */ /* __abs__ */',
    '    (inquiry) ${nb_bool}, /* inquiry nb_bool; */ /* __bool__ */',
    '    0,               /* unaryfunc nb_invert; */ /* __invert__ */',
    '    0,               /* binaryfunc nb_lshift; */ /* __lshift__ */',
    '    0,               /* binaryfunc nb_rshift; */ /* __rshift__ */',
    '    (binaryfunc) ${nb_and}, /* binaryfunc nb_and; */ /* __and__ */',
    '    0,               /* binaryfunc nb_xor; */ /* __xor__ */',
    '    (binaryfunc) ${nb_or}, /* binaryfunc nb_or; */ /* __or__ */',
    '    (unaryfunc) ${nb_int}, /* unaryfunc nb_int; */ /* __int__ */',
    '    0,               /* void *nb_reserved; */',
    '    (unaryfunc) ${nb_float},       /* unaryfunc nb_float; */ /* __float__ */',
    '    (binaryfunc) ${nb_inplace_add},       /* binaryfunc nb_inplace_add */',
    '    (binaryfunc) ${nb_inplace_subtract},       /* binaryfunc nb_inplace_subtract */',
    '    (binaryfunc) ${nb_inplace_multiply},       /* binaryfunc nb_inplace_multiply */',
    '    0, /* binaryfunc nb_inplace_remainder;  */',
    '    0, /* ternaryfunc nb_inplace_power;     */',
    '    0, /* binaryfunc nb_inplace_lshift;     */',
    '    0, /* binaryfunc nb_inplace_rshift;     */',
    '    0, /* binaryfunc nb_inplace_and;        */',
    '    0, /* binaryfunc nb_inplace_xor;        */',
    '    0, /* binaryfunc nb_inplace_or;         */',
    '    (binaryfunc) ${nb_floor_divide}, /* binaryfunc nb_floor_divide;       */',
    '    (binaryfunc) ${nb_true_divide}, /* binaryfunc nb_true_divide;         */ /* __div__ */',
    '    0, /* binaryfunc nb_inplace_floor_divide;*/',
    '    (binaryfunc) ${nb_inplace_true_divide}, /* binaryfunc nb_inplace_true_divide  */ /* __idiv__ */',
    '};'
]

TP_AS_NUMBER_TEMPLATE_OPERATOR = {
    '+': 'nb_add',
    '-': 'nb_subtract',
    '*': 'nb_multiply',
    '%': 'nb_remainder',
    'neg': 'nb_negative',
    'bool': 'nb_bool',
    '&&': 'nb_and',
    '||': 'nb_or',
    'int': 'nb_int',
    'float': 'nb_float',
    '+=': 'nb_inplace_add',
    '-=': 'nb_inplace_subtract',
    '*=': 'nb_inplace_multiply',
    '//': 'nb_floor_divide',
    '/': 'nb_true_divide',
    '/=': 'nb_inplace_true_divide'
}

NO_STATIC_OPS = {'neg', 'int', 'float', '+=', '-=', '*=', '/='}

RICH_COMPARE_OPS = {
    '<': 'Py_LT',
    '<=': 'Py_LE',
    '==': 'Py_EQ',
    '!=': 'Py_NE',
    '>': 'Py_GT',
    '>=': 'Py_GE'
}

ANNOTATION_PATTERN = r'(?:\s*(?://)?\s*\[\[[^]]+\]\])*'
AUTOBIND_CONTANT_PATTERN = re.compile(r'\[\[script::bindings::constant\(([\w\d_]+)\s*=\s*([^;\r\n]*)\)\]\]')
AUTOBIND_ENUMERATION_PATTERN = re.compile(r'\[\[script::bindings::enumeration\]\]\s*enum\s+(\w+)\s*\{([^}]+)};')
AUTOBIND_PROPERTY_PATTERN = re.compile(r'\[\[script::bindings::property\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_GETPROP_PATTERN = re.compile(r'\[\[script::bindings::getprop\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_LOADER_PATTERN = re.compile(r'\[\[script::bindings::loader\]\]\s+template<typename T>\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;{]*\{')
AUTOBIND_METHOD_PATTERN = re.compile(r'\[\[script::bindings::(auto|classmethod|constructor)\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_OPERATOR_PATTERN = re.compile(r'\[\[script::bindings::operator\(([^)]+)\)\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_CLASS_PATTERN = re.compile(r'\[\[script::bindings::class\(([^)]+)\)\]\]')
AUTOBIND_ANNOTATION_PATTERN = re.compile(r'\[\[script::bindings::(auto|container)\]\]%s\s+class\s+([^{\r\n]+)\s*{' % ANNOTATION_PATTERN)
AUTOBIND_META_PATTERN = re.compile(r'\[\[script::bindings::meta\(([^)]+)\([^)]*\)\)\]\]')

BUILDABLE_PATTERN = re.compile(r'\[\[plugin::(?:builder|resource-loader)[^\]]*\]\]\s+class\s+[\w\d_]+\s*:\s*public\s+Builder<([^{]+)>\s*\{')

CLASS_DELIMITER = '\n//%s\n' % ('-' * 120)
TYPE_DEFINED_SP = ('document', 'element', 'attribute', 'bitmap')
TYPE_DEFINED_OBJ = ('V', 'V2', 'V3', 'V4')
ARK_CORE_BUILDABLES = {'AudioPlayer', 'Object'}

INDENT = '    '

_just_print_flag = True

cwd = os.getcwd()


def make_abs_path(p):
    return (p if path.isabs(p) else path.join(cwd, p)).replace('\\', '/')


def text_align(lines, text):
    text_pos = [i.find(text) for i in lines]
    max_alignment = max(text_pos)

    return [i[:j] + ' ' * (max_alignment - j) + i[j:] if j >= 0 else i for i, j in zip(lines, text_pos)]


def gen_cmakelist_source(arguments, paths, output_dir, output_file, results):
    output_files = [make_abs_path(path.join(output_dir, output_file + i)) for i in ('.h', '.cpp')]
    dependencies = []
    macro_calls = []
    for k, genclass in results.items():
        generated_unit = [make_abs_path(path.join(output_dir, genclass.py_src_name + '.h')),
                          make_abs_path(path.join(output_dir, genclass.py_src_name + '.cpp'))]
        generated = [genclass.filename, make_abs_path(genclass.filename)] + generated_unit
        dependencies.extend(generated_unit)
        macro_calls.append(generated)
    calls = sorted(['ark_add_py_binding(%s)' % ' '.join(i) for i in macro_calls])
    arguments_without_d = ' '.join('-%s %s' % (j, k) for j, k in arguments.items() if j not in ('c',))
    arguments_without_o = ' '.join('-%s %s' % (j, k) for j, k in arguments.items() if j not in ('c', 'o'))
    return acg.format('''macro(ark_add_py_binding SRC_PATH SRC_ABS_PATH)
    add_custom_command(OUTPUT #{ARGN}
        COMMAND python ${script_path} -b "${bindable_paths}" ${arguments_without_o} #{SRC_PATH}
        DEPENDS #{SRC_ABS_PATH} ${script_path}
        WORKING_DIRECTORY ${working_dir})
    if(MSVC)
        foreach(i #{ARGN})
           set_source_files_properties(#{i} PROPERTIES COMPILE_FLAGS /bigobj)
        endforeach()
    endif()
    list(APPEND LOCAL_GENERATED_SRC_LIST #{ARGN})
endmacro()

set(LOCAL_PY_TYPE_DEPENDENCY_LIST
    %s
)

%s

add_custom_command(OUTPUT ${output_files}
        COMMAND python ${script_path} ${arguments_without_d} ${paths}
        DEPENDS #{LOCAL_PY_TYPE_DEPENDENCY_LIST}  ${script_path}
        WORKING_DIRECTORY ${working_dir})
list(APPEND LOCAL_GENERATED_SRC_LIST ${output_files})

''', script_path=make_abs_path(sys.argv[0]),
                      output_files=' '.join(output_files),
                      paths=' '.join(paths),
                      bindable_paths=path.pathsep.join(paths),
                      working_dir=os.getcwd().replace('\\', '/'),
                      arguments_without_o=arguments_without_o,
                      arguments_without_d=arguments_without_d
                      ).replace('#', '$') % ('\n    '.join(sorted(dependencies)),
                                             '\n'.join(calls))


def gen_header_source(filename, output_dir, output_file, results, namespaces):
    filedir, name = path.split(filename)
    declares = ['\nvoid __init_%s__(PyObject* module);' % name]
    includes = []
    for k, genclass in results.items():
        if output_dir is None:
            declares.append(CLASS_DELIMITER)
            gen_class_header_source(genclass, declares)
        else:
            includes.append('#include "%s.h"' % genclass.py_src_name)
            if output_file is None:
                local_declares = []
                gen_class_header_source(genclass, local_declares)
                hfilepath = path.join(output_dir, genclass.py_src_name + '.h')
                acg.write_to_file(_just_print_flag and hfilepath, gen_py_binding_h(hfilepath, namespaces, [], local_declares))
    return gen_py_binding_h(filename, namespaces, includes, declares)


def gen_class_header_source(genclass, declares):
    method_declarations = [i.gen_declaration() for i in genclass.methods]
    s = '\n    '.join(i for i in method_declarations if i)
    declares.append(acg.format('''class ${py_class_name} : public ark::plugin::python::PyArkType {
public:
    ${py_class_name}(const String& name, const String& doc, long flags);

    ${method_declares}
};''', method_declares=s, py_class_name=genclass.py_class_name))


def gen_py_binding_h(filename, namespaces, includes, declares):
    header_macro = re.sub(r'[^\w\d]', '_', filename).upper()
    return acg.format('''#ifndef PY_${header_macro}_GEN
#define PY_${header_macro}_GEN

${includes}

#include "python/extension/py_ark_type.h"

${0}

#endif''', acg.wrapByNamespaces(namespaces, '\n'.join(declares)), header_macro=header_macro,
                      includes='\n'.join(includes))


def gen_body_source(filename, output_dir, output_file, namespaces, modulename, results, buildables):
    filedir, name = path.split(filename)
    includes = []
    lines = []
    for k, genclass in results.items():
        if output_dir is None:
            lines.append(CLASS_DELIMITER)
            gen_class_body_source(genclass, includes, lines, buildables)
        else:
            includes.append('#include "%s"' % genclass.filename)
            if output_file is None:
                classincludes = []
                classlines = []
                gen_class_body_source(genclass, classincludes, classlines, buildables)
                acg.write_to_file(_just_print_flag and path.join(output_dir, genclass.py_src_name + '.cpp'),
                                  gen_py_binding_cpp(genclass.py_src_name, namespaces, classincludes, classlines))

    add_types = '\n    '.join('pi->pyModuleAddType<%s, %s>(module, "%s", "%s", Py_TPFLAGS_DEFAULT%s);' % (i.py_class_name, i.binding_classname, modulename, i.binding_classname, '|Py_TPFLAGS_HAVE_GC' if i.is_container else '') for i in results.values())
    lines.append('\n' + '''void __init_%s__(PyObject* module)
{
    const sp<ark::plugin::python::PythonInterpreter>& pi = ark::plugin::python::PythonInterpreter::instance();
    %s
}''' % (name, add_types))

    return gen_py_binding_cpp(name, namespaces, includes, lines)


def gen_class_body_source(genclass, includes, lines, buildables):
    tp_method_lines = []
    includes.append('#include "%s"' % genclass.filename)
    if genclass.has_methods():
        tp_method_lines.append('PyTypeObject* pyTypeObject = getPyTypeObject();')
        method_defs = [i.gen_py_method_def(genclass) for i in genclass.methods]
        methoddeclare = ',\n    '.join(i for i in method_defs if i)
        if methoddeclare:
            lines.append('\nstatic PyMethodDef %s_methods[] = {\n    %s,\n    {NULL}\n};' % (
                genclass.py_class_name, methoddeclare))
            tp_method_lines.append('pyTypeObject->tp_methods = %s_methods;' % genclass.py_class_name)

        getprop_methods = genclass.getprop_methods()
        if getprop_methods:
            if len(getprop_methods) > 1:
                print("WARNING: at least two getprop methods defined in %s, which is probably not right", genclass.classname)
            tp_method_lines.append('pyTypeObject->tp_getattro = (getattrofunc) %s;' % getprop_methods[0].name)

        operator_defs = genclass.gen_operator_defs()
        if operator_defs:
            lines.append('')
            lines.extend(text_align(text_align(operator_defs, '/*'), '*/'))
            tp_method_lines.append('pyTypeObject->tp_as_number = &%s_tp_as_number;' % genclass.py_class_name)

        rich_compare_defs = genclass.gen_rich_compare_defs()
        if rich_compare_defs:
            lines.extend(rich_compare_defs)
            tp_method_lines.append('pyTypeObject->tp_richcompare = reinterpret_cast<richcmpfunc>(%s_tp_richcompare);' % genclass.py_class_name)

        property_defs = genclass.gen_property_defs()
        if property_defs:
            propertydeclare = '{"%s", %s, %s, "%s"}'
            methoddeclare = ',\n    '.join(propertydeclare % tuple(i) for i in property_defs)
            lines.append('\nstatic PyGetSetDef %s_getseters[] = {\n    %s,\n    {NULL}\n};' % (
            genclass.py_class_name, methoddeclare))
            tp_method_lines.append('pyTypeObject->tp_getset = %s_getseters;' % genclass.py_class_name)

        for i in genclass.loader_methods():
            lines.append(acg.format(LOADER_TEMPLATE, classname=genclass.classname, methodname=i.name))
            tp_method_lines.append('{')
            tp_method_lines.append(INDENT + 'std::map<TypeId, LoaderFunction>& loader = ensureLoader("%s");' % i.name)
            for j in buildables:
                if j.find('::') == -1:
                    tp_method_lines.append(INDENT + i.gen_loader_statement(genclass.classname, j, i.name))
            tp_method_lines.append('}')

    genclass.gen_py_type_constructor_codes(tp_method_lines)
    if genclass.constants:
        tp_method_lines.extend('_constants["%s"] = %s;' % (i, j) for i, j in genclass.constants.items())
    constructor = acg.format('''${py_class_name}::${py_class_name}(const String& name, const String& doc, long flags)
    : PyArkType(name, doc, flags)
{${tp_methods}
}''', py_class_name=genclass.py_class_name, tp_methods='\n    ' + '\n    '.join(tp_method_lines) if tp_method_lines else '')
    methoddefinition = acg.format('''%s ${py_class_name}::%s(%s)
{
    %s
}''', py_class_name=genclass.py_class_name)
    lines.append('\n' + constructor)
    method_definitions = [(i, i.gen_definition(genclass)) for i in genclass.methods]
    lines.extend(
        '\n' + methoddefinition % (i.gen_py_return(), i.name, i.gen_py_arguments(), j) for i, j in method_definitions if
        j)


def gen_py_binding_cpp(name, namespaces, includes, lines):
    return ('''#include "%s.h"

#include "core/ark.h"

#include "python/extension/arkmodule.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_ark_meta_type.h"
#include "python/extension/py_container.h"

%s

using namespace ark::plugin::python;

''' % (name, '\n'.join(includes))) + acg.wrapByNamespaces(namespaces, '\n'.join(lines))


class GenArgumentMeta:
    def __init__(self, typename, castsig, parsetuplesig, is_base_type=False):
        self._typename = typename
        self._cast_signature = castsig
        self._parse_signature = parsetuplesig
        self._is_base_type = is_base_type

    @property
    def typename(self):
        return self._typename

    @property
    def cast_signature(self):
        return self._cast_signature

    @property
    def parse_signature(self):
        return self._parse_signature

    @property
    def is_base_type(self):
        return self._is_base_type


class GenConverter:
    def __init__(self, check_func, cast_func):
        self._check_func = check_func
        self._cast_func = cast_func

    def check(self, var):
        return self._check_func.format(var)

    def cast(self, var):
        return self._cast_func.format(var)


class GenArgument:
    def __init__(self, accept_type, default_value, meta, _str):
        self._accept_type = accept_type
        self._default_value = default_value
        self._meta = meta
        self._str = _str

    @property
    def accept_type(self):
        return self._accept_type

    @property
    def default_value(self):
        return self._default_value

    @property
    def meta(self):
        return self._meta

    @property
    def typename(self):
        return self._meta.typename

    @property
    def parse_signature(self):
        s = self._meta.parse_signature
        return s if self._default_value is None else '|' + s

    def type_compare(self, typename):
        return acg.typeCompare(typename, self._str)

    def gen_type_check(self, varname):
        if self._accept_type in ARK_PY_ARGUMENT_CHECKERS:
            return "%s && %s" % (varname, ARK_PY_ARGUMENT_CHECKERS[self._accept_type].check(varname))
        return None

    def gen_declare(self, objname, argname, extract_cast=False):
        typename = self._meta.cast_signature
        if self._meta.is_base_type:
            return '%s %s = %s;' % (typename, objname, gen_cast_call(typename, argname))
        if self.typename == self._meta.cast_signature:
            return '%s %s = %s;' % (typename, objname, argname)
        m = acg.getSharedPtrType(self._accept_type)
        if m == 'Scope':
            return acg.format('const sp<Scope> ${objname} = PythonInterpreter::instance()->toScope(kws);',
                              objname=objname, argname=argname)
        if m == 'char*':
            return self._gen_var_declare('String', objname, 'toType', 'String', argname)
        if m in TYPE_DEFINED_OBJ:
            return self._gen_var_declare(m, objname, 'toType', m, argname)
        if m in TYPE_DEFINED_SP:
            return self._gen_var_declare(m, objname, 'toCppObject', m, argname)
        if m != self._accept_type:
            return self._gen_var_declare('sp<%s>' % m, objname, 'asInterfaceOrNull' if extract_cast else 'toSharedPtr', m, argname)
        return self._gen_var_declare(typename, objname, 'toType', typename, argname)

    def _gen_var_declare(self, typename, varname, funcname, functype, argname):
        if self._default_value and self._accept_type.startswith('sp<'):
            return 'const %s %s = %s ? PythonInterpreter::instance()->%s<%s>(%s) : %s;' % (
                typename, varname, argname, funcname, functype, argname, self._default_value)
        return 'const %s %s = PythonInterpreter::instance()->%s<%s>(%s);' % (typename, varname, funcname, functype, argname)

    def str(self):
        return self._str

    def __str__(self):
        return self.str()


ARK_PY_ARGUMENTS = (
    (r'String\s*&?', GenArgumentMeta('const char*', 'const char*', 's')),
    (r'sp<Scope>\s*&', GenArgumentMeta('PyObject*', 'sp<Scope>', '')),
    (r'std::wstring\s*&', GenArgumentMeta('PyObject*', 'std::wstring', 'O')),
    (r'Box\s*&', GenArgumentMeta('PyObject*', 'Box', 'O')),
    (r'sp<([^>]+|[\w\d_]+<[\w\d_]+>)>\s*&', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'(document|element|attribute)\s*&', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (r'V2', GenArgumentMeta('PyObject*', 'V2', 'O')),
    (r'V3', GenArgumentMeta('PyObject*', 'V3', 'O')),
    (r'([^>]+|[\w\d_]+<[\w\d_]+>)\s*&', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'(uint32_t|unsigned int|uint8_t)', GenArgumentMeta('uint32_t', 'uint32_t', 'I')),
    (r'(int32_t|int|int8_t)', GenArgumentMeta('int32_t', 'int32_t', 'i')),
    (r'float', GenArgumentMeta('float', 'float', 'f')),
    (r'bool', GenArgumentMeta('int32_t', 'bool', 'p', True)),
    (r'[^:]+::.+', GenArgumentMeta('int32_t', 'int32_t', 'i')),
    (r'TypeId', GenArgumentMeta('PyObject*', 'TypeId', 'O')),
)

ARK_PY_ARGUMENT_CHECKERS = {
    'bool': GenConverter('(PyLong_CheckExact({0}) || PyBool_Check({0}))', 'PyLong_AsLong({0}) != 0'),
    'int32_t': GenConverter('(PyLong_CheckExact({0}) || PyFloat_CheckExact({0}))', 'PyLong_AsLong({0})'),
    'uint32_t': GenConverter('(PyLong_CheckExact({0}) || PyFloat_CheckExact({0}))', 'PyLong_AsLong({0})'),
    'float': GenConverter('(PyLong_CheckExact({0}) || PyFloat_CheckExact({0}))', 'PyFloat_AsDouble({0})'),
    'V2': GenConverter('(PyTuple_CheckExact({0}) && PyObject_Length({0}) == 2)', '{0}'),
    'V3': GenConverter('(PyTuple_CheckExact({0}) && PyObject_Length({0}) == 3)', '{0}'),
    'std::wstring': GenConverter('PyUnicode_CheckExact({0})', 'PyUnicode_DATA({0})')
}


def parse_method_arguments(arguments):
    args = []
    for i, j in enumerate(arguments):
        m = None
        for k, l in ARK_PY_ARGUMENTS:
            argstr = acg.strip_key_words(j, ['const', '&'])
            m = re.match(k, argstr)
            if m:
                default_value = None
                pos = argstr.find('=')
                if pos != -1 and not argstr.startswith('sp<Scope>'):
                    default_value = argstr[pos + 1:].strip()
                    if default_value.endswith('('):
                        default_value += ')'
                accept_type = acg.format(l.cast_signature, *m.groups())
                args.append(GenArgument(accept_type, default_value, l, argstr))
                break
        if not m:
            print('Undefined method argument: "%s"' % arguments[i])
            sys.exit(-1)
    return args


def gen_method_call_arg(i, arg, argtype):
    pos = arg.find('=')
    if pos != -1:
        arg = arg[0:pos]
    targettype = ' '.join(arg.split()[:-1])
    equals = acg.typeCompare(targettype, argtype)
    return 'obj%d' % i if equals or '<' in argtype else gen_cast_call(targettype, 'obj%d' % i)


def gen_cast_call(targettype, name):
    if targettype == 'bool':
        return '%s != 0' % name
    return 'static_cast<%s>(%s)' % (targettype, name)


class GenMethod(object):
    def __init__(self, name, args, return_type):
        self._name = name
        self._return_type = return_type
        self._arguments = parse_method_arguments(args)
        self._has_keyvalue_arguments = args and self._arguments[-1].type_compare('sp<Scope>')
        self._flags = ('METH_VARARGS|METH_KEYWORDS' if self._has_keyvalue_arguments else 'METH_VARARGS')
        self._self_argument = None

    def gen_py_method_def(self, genclass):
        return None

    def gen_py_return(self):
        return 'PyObject*'

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* args' + (', PyObject* kws' if self._has_keyvalue_arguments else '')

    def gen_local_var_declarations(self):
        declares = {}
        for i, j in enumerate(self._arguments):
            typename = j.typename
            if typename in declares:
                declares[typename] = '%s, %sarg%d' % (declares[typename], '*' if typename.endswith('*') else '', i)
            else:
                declares[typename] = '%s arg%d' % (typename, i)
            if j.default_value is not None:
                dmap = {'true': '1', 'false': '0'}
                if j.typename in ('uint32_t', 'int32_t', 'float'):
                    dvalue = j.default_value if j.default_value not in dmap else dmap[j.default_value]
                elif typename == 'PyObject*' and j.accept_type == 'bool':
                    dvalue = 'Py_True' if j.default_value == 'true' else 'Py_False'
                elif typename == 'bool' and j.accept_type == 'bool':
                    dvalue = 'true' if j.default_value == 'true' else 'false'
                else:
                    dvalue = 'nullptr'
                declares[typename] += ' = %s' % dvalue
            elif not j.meta._parse_signature:
                declares[typename] += ' = kws'
        return [i + ';' for i in declares.values()]

    @property
    def name(self):
        return self._name

    @property
    def arguments(self):
        return self._arguments

    @property
    def return_type(self):
        return self._return_type

    @property
    def err_return_value(self):
        return 'Py_RETURN_NONE'

    @property
    def check_argument_type(self):
        return True

    def _gen_body_lines(self, genclass):
        return []

    def _gen_call_statement(self, genclass, argnames):
        return 'unpacked->%s(%s);' % (self._name, argnames)

    @staticmethod
    def gen_return_statement(return_type, py_return):
        if acg.typeCompare(return_type, py_return):
            return ['return ret;']

        if py_return != 'PyObject*':
            return ['return %s;' % gen_cast_call(py_return, 'ret')]

        if return_type == 'void':
            return ['Py_RETURN_NONE;']
        m = acg.getSharedPtrType(return_type)
        if m in ('std::wstring',):
            fromcall = 'template fromType<%s>' % m
        else:
            fromcall = 'toPyObject'
        return ['return PythonInterpreter::instance()->%s(ret);' % fromcall]

    def _gen_parse_tuple_code(self, lines, declares, args):
        parse_format = ''.join(i.parse_signature for i in args)
        if parse_format.count('|') > 1:
            ts = parse_format.split('|')
            if any(len(i) > 1 for i in ts[1:]):
                print('Illegal default arguments: %s', parse_format)
                sys.exit(-1)
            parse_format = ts[0] + '|' + ''.join(ts[1:])
        parsestatement = '''%s
    if(!PyArg_ParseTuple(args, "%s", %s))
        %s;
''' % ('\n    '.join(declares), parse_format, ', '.join('&arg%d' % i for i, j in enumerate(args) if j.meta._parse_signature), self.err_return_value)
        lines.append(parsestatement)

    def need_unpack_statement(self):
        return True

    def gen_py_type_constructor_codes(self, lines, genclass):
        pass

    def gen_self_statement(self, genclass):
        cast_statement = ''
        if self._self_argument and not self._self_argument.type_compare('sp<%s>' % genclass.binding_classname):
            cast_statement = '.cast<%s>()' % acg.getSharedPtrType(self._self_argument.accept_type)
        return 'unpacked' + cast_statement

    def gen_declaration(self):
        return 'static %s %s(%s);' % (self.gen_py_return(), self._name, self.gen_py_arguments())

    @staticmethod
    def _gen_convert_args_code(lines, argdeclare):
        if argdeclare:
            lines.extend(argdeclare)

    def gen_definition(self, genclass):
        lines = []

        if len(self._arguments) > 0:
            self._gen_parse_tuple_code(lines, self.gen_local_var_declarations(), self._arguments)

        if self.need_unpack_statement():
            lines.append(acg.format('const sp<${class_name}>& unpacked = self->unpack<${class_name}>();',
                                    class_name=genclass.binding_classname))
        self.gen_definition_body(genclass, lines, self._arguments, [None] * len(self._arguments))

        return '\n    '.join(lines)

    def gen_definition_body(self, genclass, lines, not_overloaded_args, gen_type_check_args, exact_cast=False):
        bodylines = self._gen_body_lines(genclass)
        args = [(i, j) for i, j in enumerate(not_overloaded_args) if j]
        argdeclare = [j.gen_declare('obj%d' % i, 'arg%d' % i, exact_cast) for i, j in args]
        self_type_checks = []
        if self._self_argument:
            if not self._self_argument.type_compare('sp<%s>' % genclass.binding_classname):
                self_type_checks.append('unpacked.is<%s>()' % acg.getSharedPtrType(self._self_argument.accept_type))
        self._gen_convert_args_code(bodylines, argdeclare)

        r = acg.strip_key_words(self._return_type, ['virtual', 'const', '&'])
        argtypes = [i.gen_declare('t', 't').split()[0] for i in self._arguments]
        argnames = ', '.join(gen_method_call_arg(i, j.str(), argtypes[i]) for i, j in enumerate(self._arguments))
        callstatement = self._gen_call_statement(genclass, argnames)
        py_return = self.gen_py_return()
        pyret = ['return 0;'] if py_return == 'int' else self.gen_return_statement(r, py_return)
        if r != 'void' and r:
            callstatement = '%s ret = %s' % (acg.strip_key_words(self._return_type, ['virtual']), callstatement)
        calling_lines = [callstatement] + pyret
        type_checks = [(i, j.gen_type_check('t')) for i, j in enumerate(gen_type_check_args) if j]
        nullptr_check = ['obj%d' % i for i, j in type_checks if not j]
        if self_type_checks or nullptr_check:
            nullptr_check = self_type_checks + nullptr_check
            calling_lines = ['if(%s)' % ' && '.join(nullptr_check), '{'] + [INDENT + i for i in calling_lines] + ['}']
        lines.extend(bodylines + calling_lines)

    @staticmethod
    def split(repl):
        name = repl[0].split()[-1]
        rs = repl[0].split()[:-1]
        return_type = ' '.join(i for i in rs if i not in('static', 'ARK_API'))
        args = [i.strip() for i in repl[1].split(',') if i]
        return name, args, return_type, 'static' in rs


class GenConstructorMethod(GenMethod):
    def __init__(self, name, args, is_static):
        GenMethod.__init__(self, '__init__', args, '')
        self._funcname = name
        self._is_static = is_static

    def _gen_call_statement(self, genclass, argnames):
        if self._is_static:
            return 'self->box = new Box(%s::%s(%s).pack());' % (genclass.classname, self._funcname, argnames)
        return 'self->box = new Box(sp<%s>::make(%s).pack());' % (self._funcname, argnames)

    def gen_py_return(self):
        return 'int'

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* args, PyObject* /*kws*/'

    @property
    def err_return_value(self):
        return 'return -1'

    def need_unpack_statement(self):
        return False

    def gen_py_type_constructor_codes(self, lines, genclass):
        lines.append('pyTypeObject->tp_init = reinterpret_cast<initproc>(%s::__init__);' % genclass.py_class_name)

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenConstructorMethod)(m1, m2)


class GenMetaMethod(GenMethod):
    def __init__(self, name):
        GenMethod.__init__(self, name, [], '')

    def gen_declaration(self):
        return None

    def gen_definition(self, classname):
        return None

    def gen_py_method_def(self, genclass):
        return '{"%s", (PyCFunction) PyArkMetaType<%s>::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), genclass.binding_classname, self._name, self._flags)


class GenPropertyMethod(GenMethod):
    def __init__(self, name, args, return_type, is_static):
        GenMethod.__init__(self, name, args, return_type)
        if is_static:
            self._self_argument = self._arguments and self._arguments[0]
            self._arguments = self._arguments[1:]
        self._is_static = is_static
        self._is_setter = name.startswith('set')
        self._property_name = name[3].lower() + name[4:] if self._is_setter else name

    def gen_py_return(self):
        return 'int' if self._is_setter else "PyObject*"

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    @property
    def is_static(self):
        return self._is_static

    def _gen_convert_args_code(self, lines, argdeclare):
        if argdeclare:
            arg0 = self._arguments[0]
            meta = GenArgumentMeta('PyObject*', arg0.accept_type, 'O')
            ga = GenArgument(arg0.accept_type, arg0.default_value, meta, str(arg0))
            lines.append(ga.gen_declare('obj0', 'arg0'))

    def gen_py_arguments(self):
        if self._is_setter:
            return 'Instance* self, PyObject* arg0, void* /*closure*/'
        return 'Instance* self, PyObject* /*args*/'

    def gen_py_getset_def(self, properties, genclass):
        property_def = self._ensure_property_def(properties)
        if self._is_setter:
            property_def[2] = '(setter) %s::%s' % (genclass.py_class_name, self._name)
        else:
            property_def[1] = '(getter) %s::%s' % (genclass.py_class_name, self._name)

    def _gen_call_statement(self, genclass, argnames):
        self_statement = self.gen_self_statement(genclass)
        if self._is_static:
            lines = ['%s::%s(%s%s);' % (genclass.classname, self._name, self_statement, argnames and ', ' + argnames)]
        else:
            lines = ['%s->%s(%s);' % (self_statement, self._name, argnames)]
        if self._is_setter and genclass.is_container:
            lines.append('self->addObjectToContainer(obj0);')
        return '\n    '.join(lines)

    def _ensure_property_def(self, properties):
        snake_property_name = acg.camel_case_to_snake_case(self._property_name)
        for i in properties:
            if i[0] == snake_property_name:
                return i
        property_def = [snake_property_name, 'nullptr', 'nullptr', self._property_name]
        properties.append(property_def)
        return property_def

    @property
    def err_return_value(self):
        return 'return -1'

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenPropertyMethod, is_static=m1.is_static)(m1, m2)


class GenGetPropMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    def _gen_convert_args_code(self, lines, argdeclare):
        if argdeclare:
            arg0 = self._arguments[0]
            meta = GenArgumentMeta('PyObject*', arg0.accept_type, 'O')
            ga = GenArgument(arg0.accept_type, arg0.default_value, meta, str(arg0))
            lines.append(ga.gen_declare('obj0', 'arg0'))

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* arg0'

    def _gen_body_lines(self, genclass):
        return ['PyObject* attr = PyObject_GenericGetAttr(reinterpret_cast<PyObject*>(self), arg0);',
                'if(attr)',
                '    return attr;',
                'if(!PythonInterpreter::instance()->exceptErr(PyExc_AttributeError))',
                '	Py_RETURN_NONE;'
                '\n']


class GenLoaderMethod(GenMethod):
    def __init__(self, name, args):
        GenMethod.__init__(self, name, ['TypeId typeId'] + args, 'PyObject*')

    def gen_py_method_def(self, genclass):
        return '{"%s", (PyCFunction) %s::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), genclass.py_class_name, self._name, self._flags)

    def _gen_call_statement(self, genclass, argnames):
        return 'PythonInterpreter::instance()->ensurePyArkType(reinterpret_cast<PyObject*>(self))->load(*self, "%s", %s);' % (self._name, argnames)

    def need_unpack_statement(self):
        return False

    def gen_loader_statement(self, classname, beanname, methodname):
        return acg.format('loader[Type<${beanname}>::id()] = PyArk${classname}_${methodname}Function<${beanname}>;', classname=classname, beanname=beanname, methodname=methodname)


class GenMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_method_def(self, genclass):
        return '{"%s", (PyCFunction) %s::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), genclass.py_class_name, self._name, self._flags)

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenMemberMethod)(m1, m2)


class GenStaticMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_method_def(self, genclass):
        return '{"%s", (PyCFunction) %s::%s, %s|METH_STATIC, nullptr}' % (acg.camel_case_to_snake_case(self._name), genclass.py_class_name, self._name, self._flags)

    def need_unpack_statement(self):
        return False

    def _gen_call_statement(self, genclass, argnames):
        return '%s::%s(%s);' % (genclass.classname, self._name, argnames)

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenStaticMethod)(m1, m2)


class GenStaticMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)
        self._self_argument = self._arguments and self._arguments[0]
        self._arguments = self._arguments[1:]

    def gen_py_method_def(self, genclass):
        return '{"%s", (PyCFunction) %s::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), genclass.py_class_name, self._name, self._flags)

    def _gen_call_statement(self, genclass, argnames):
        return '%s::%s(%s%s);' % (genclass.classname, self._name, self.gen_self_statement(genclass), argnames if not argnames else ', ' + argnames)

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenStaticMemberMethod)(m1, m2)


class GenOperatorMethod(GenMethod):
    def __init__(self, name, args, return_type, operator):
        GenMethod.__init__(self, name, args, return_type)
        self._is_static = operator not in NO_STATIC_OPS
        self._self_argument = None if self._is_static else self._arguments and self._arguments[0]
        self._arguments = self._arguments if self._is_static else self._arguments[1:]
        self._operator = operator

    def gen_py_return(self):
        if self._return_type == 'bool':
            return 'int32_t'
        return 'PyObject*'

    def need_unpack_statement(self):
        return not self._is_static

    @property
    def check_argument_type(self):
        return False

    def gen_py_arguments(self):
        arglen = len(self._arguments)
        args = ['PyObject* arg%d' % i for i in range(arglen)]
        if self._is_static:
            return ', '.join(args)
        return ', '.join(['Instance* self'] + args)

    def _gen_call_statement(self, genclass, argnames):
        if self._is_static:
            return '%s::%s(%s);' % (genclass.classname, self._name, argnames)
        return '%s::%s(%s%s);' % (genclass.classname, self._name, self.gen_self_statement(genclass), argnames if not argnames else ', ' + argnames)

    def gen_return_statement(self, return_type, py_return):
        if self._operator in ('+=', '-=', '*=', '/='):
            return ['Py_INCREF(self);', 'return reinterpret_cast<PyObject*>(self);']
        return GenMethod.gen_return_statement(return_type, py_return)

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    @property
    def err_return_value(self):
        return 'Py_RETURN_NOTIMPLEMENTED'

    @property
    def operator(self):
        return self._operator

    @staticmethod
    def overload(m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenOperatorMethod, operator=m1.operator)(m1, m2)


class GenRichCompareMethod(GenMethod):
    def __init__(self, name, args, return_type, operator):
        GenMethod.__init__(self, name, args, return_type)
        self._self_argument = self._arguments and self._arguments[0]
        self._arguments = self._arguments[1:]
        self._operator = operator

    def gen_py_return(self):
        return 'PyObject*'

    def _gen_call_statement(self, genclass, argnames):
        return '%s::%s(%s%s);' % (genclass.classname, self._name, self.gen_self_statement(genclass), argnames if not argnames else ', ' + argnames)

    @staticmethod
    def _gen_convert_args_code(lines, argdeclare):
        pass

    def _gen_parse_tuple_code(self, lines, declares, args):
        if args:
            meta = GenArgumentMeta('PyObject*', args[0].accept_type, 'O')
            ga = GenArgument(args[0].accept_type, args[0].default_value, meta, str(args[0]))
            lines.append(ga.gen_declare('obj0', 'args'))

    @property
    def operator(self):
        return self._operator


def create_overloaded_method_type(base_type, **kwargs):

    class GenOverloadedMethod(base_type):
        def __init__(self, m1, m2):
            base_type.__init__(self, m1.name, [], m1.return_type, **kwargs)
            self._arguments = self._replace_arguments(m1.arguments)
            self._overloaded_methods = []
            self.add_overloaded_method(m1)
            self.add_overloaded_method(m2)

        def gen_definition_body(self, genclass, lines, arguments, gen_type_check_args):
            not_overloaded_args = [i for i in self._arguments]

            for i in self._overloaded_methods:
                for j, k in enumerate(not_overloaded_args):
                    if k and i.arguments[j].accept_type != k.accept_type:
                        not_overloaded_args[j] = None

            m0 = self._overloaded_methods[0]
            not_overloaded_names = ['obj%d' % i for i, j in enumerate(not_overloaded_args) if j and j.default_value is None]
            not_overloaded_declar = [j.gen_declare('obj%d' % i, 'arg%d' % i, not m0.check_argument_type) for i, j in enumerate(not_overloaded_args) if j]
            self._gen_convert_args_code(lines, not_overloaded_declar)
            for i in self._overloaded_methods:
                type_checks = [k.gen_type_check('arg%d' % j) for j, k, l in zip(range(len(i.arguments)), i.arguments, not_overloaded_args) if not l]
                lines.extend(['if(%s)' % ' && '.join(([j for j in type_checks if j] + not_overloaded_names) or ['true']), '{'])
                body_lines = []
                overloaded_args = [None if j else k for j, k in zip(not_overloaded_args, i.arguments)]
                i.gen_definition_body(genclass, body_lines, overloaded_args, overloaded_args, True)
                lines.extend(INDENT + j for j in body_lines)
                lines.append('}')
            return_type = m0.err_return_value
            if m0.check_argument_type:
                lines.append('PyErr_SetString(PyExc_TypeError, "Calling overloaded method(%s) failed, no arguments matched");' % self._name)
            lines.append(return_type + ';')

        def add_overloaded_method(self, method):
            if self._overloaded_methods:
                m1 = self._overloaded_methods[-1]
                if len(m1.arguments) != len(method.arguments) or len(method.arguments) == 0:
                    print('Overloaded methods(%s, %s) should have equal number of arguments' % (m1, method))
                    sys.exit(-1)
            method._arguments = self._replace_arguments(method.arguments)
            # method.calling_type_check = True
            self._overloaded_methods.append(method)

        @staticmethod
        def _replace_arguments(arguments):
            return [GenArgument(i.accept_type, i.default_value, GenArgumentMeta('PyObject*', i.meta.cast_signature, 'O'), i.str()) for i in arguments]

    return GenOverloadedMethod


class GenClass(object):
    def __init__(self, filename, class_name, is_container=False):
        self._py_src_name = 'py_ark_' + acg.camel_case_to_snake_case(class_name) + '_type'
        self._py_class_name = 'PyArk%sType' % class_name
        self._filename = filename
        self._classname = class_name
        self._binding_classname = class_name
        self._is_container = is_container
        self._methods = {}
        self._constants = {}

    @property
    def classname(self):
        return self._classname

    @property
    def binding_classname(self):
        return self._binding_classname

    @binding_classname.setter
    def binding_classname(self, v):
        self._py_src_name = 'py_ark_' + acg.camel_case_to_snake_case(v) + '_type'
        self._py_class_name = 'PyArk%sType' % v
        self._binding_classname = v

    @property
    def py_src_name(self):
        return self._py_src_name

    @property
    def py_class_name(self):
        return self._py_class_name

    @property
    def methods(self):
        return self._methods.values()

    @property
    def constants(self):
        return self._constants

    @property
    def filename(self):
        return self._filename

    @property
    def is_container(self):
        return self._is_container

    def add_method(self, method):
        try:
            m = self._methods[method.name]
            method = type(m).overload(m, method)
        except KeyError:
            pass
        self._methods[method.name] = method

    def add_constant(self, name, value):
        self._constants[name] = value

    def has_methods(self):
        return len(self._methods) > 0

    def property_methods(self):
        return self.find_methods_by_type(GenPropertyMethod)

    def loader_methods(self):
        return self.find_methods_by_type(GenLoaderMethod)

    def operator_methods(self):
        return self.find_methods_by_type(GenOperatorMethod)

    def rich_compare_methods(self):
        return self.find_methods_by_type(GenRichCompareMethod)

    def getprop_methods(self):
        return self.find_methods_by_type(GenGetPropMethod)

    def find_methods_by_type(self, method_type):
        return [i for i in self.methods if isinstance(i, method_type)]

    def gen_py_type_constructor_codes(self, lines):
        for i in self.methods:
            i.gen_py_type_constructor_codes(lines, self)

    def gen_property_defs(self):
        property_defs = []
        for i in self.property_methods():
            i.gen_py_getset_def(property_defs, self)
        return property_defs

    def gen_operator_defs(self):
        operator_defs = []
        operator_methods = self.operator_methods()
        if operator_methods:
            args = {'py_class_name': self._py_class_name}
            methods_dict = dict((i.operator, '%s::%s' % (self._py_class_name, i.name)) for i in operator_methods)
            args.update((j, methods_dict[i] if i in methods_dict else '0') for i, j in TP_AS_NUMBER_TEMPLATE_OPERATOR.items())
            operator_defs.extend(acg.format(i, **args) for i in TP_AS_NUMBER_TEMPLATE)
        return operator_defs

    def gen_rich_compare_defs(self):
        rich_compare_defs = []
        rich_compare_methods = self.rich_compare_methods()
        if rich_compare_methods:
            cases = []
            for i in rich_compare_methods:
                try:
                    cases.append('    case %s:' % RICH_COMPARE_OPS[i.operator])
                    cases.append('        return PythonInterpreter::instance()->toPyObject(%s::%s(unpacked, obj0));' % (self._classname, i.name))
                except KeyError:
                    pass
            rich_compare_defs.extend([
                '',
                'static PyObject* %s_tp_richcompare (PyArkType::Instance* self, PyObject* args, int op) {' % self._py_class_name,
                '    const sp<%s>& unpacked = self->unpack<%s>();' % (self._binding_classname, self._binding_classname),
                '    const sp<%s> obj0 = PythonInterpreter::instance()->toSharedPtr<%s>(args);' % (self._binding_classname, self._binding_classname),
                '    switch(op) {'] + cases +
                [
                '    default:',
                '        break;',
                '    }',
                '    return Py_NotImplemented;',
                '}',
            ])
        return rich_compare_defs


def get_result_class(results, filename, classname):
    genclass = results[classname] if classname in results else GenClass(filename, classname)
    if classname not in results:
        results[classname] = genclass
    return genclass


def main(params, paths):
    results = {}
    bindables = set(ARK_CORE_BUILDABLES)

    namespaces = params['p'].replace('::', ':').split(':')
    modulename = params['m']
    output_file = params['o'] if 'o' in params else None
    output_dir = params['d'] if 'd' in params else None
    output_cmakelist = params['c'] if 'c' in params else None
    bindable_paths = params['b'] if 'b' in params else None

    def automethod(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        method_modifier = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        if is_static and method_modifier == 'auto':
            genmethod = GenStaticMethod(name, args, return_type)
        elif genclass.classname == name or method_modifier == 'constructor':
            genmethod = GenConstructorMethod(name, args, is_static)
        elif is_static and method_modifier == 'classmethod':
            genmethod = GenStaticMemberMethod(name, args, return_type)
        else:
            genmethod = GenMemberMethod(name, args, return_type)
        genclass.add_method(genmethod)

    def autooperator(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        operator = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        assert is_static
        if operator in RICH_COMPARE_OPS:
            genclass.add_method(GenRichCompareMethod(name, args, return_type, operator))
        else:
            genclass.add_method(GenOperatorMethod(name, args, return_type, operator))

    def autoclass(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        genclass.binding_classname = x.strip('"')

    def autoloader(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenLoaderMethod(name, args))

    def autoproperty(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenPropertyMethod(name, args, return_type, is_static))

    def autogetprop(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenGetPropMethod(name, args, return_type))

    def autoannotation(filename, content, m, x):
        names = [i for i in x[1].split(':', 2)[0].split() if i not in ('ARK_API', 'final')]
        if len(names) >= 1:
            results[names[-1]] = GenClass(filename, names[-1], x[0].strip() == 'container')

    def autoconstant(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        genclass.add_constant(x[0], x[1])

    def autometa(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        genclass.add_method(GenMetaMethod(x))

    def autobindable(filename, content, main_class, x):
        bindables.add(x)

    def autoenumeration(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        for i in x[1].split(','):
            varname = i.split('=')[0].strip()
            if varname:
                genclass.add_constant(varname, '%s::%s' % (main_class, varname))

    acg.matchHeaderPatterns(paths,
                            HeaderPattern(AUTOBIND_ANNOTATION_PATTERN, autoannotation),
                            HeaderPattern(AUTOBIND_METHOD_PATTERN, automethod),
                            HeaderPattern(AUTOBIND_OPERATOR_PATTERN, autooperator),
                            HeaderPattern(AUTOBIND_CLASS_PATTERN, autoclass),
                            HeaderPattern(AUTOBIND_LOADER_PATTERN, autoloader),
                            HeaderPattern(AUTOBIND_PROPERTY_PATTERN, autoproperty),
                            HeaderPattern(AUTOBIND_GETPROP_PATTERN, autogetprop),
                            HeaderPattern(AUTOBIND_CONTANT_PATTERN, autoconstant),
                            HeaderPattern(AUTOBIND_ENUMERATION_PATTERN, autoenumeration),
                            HeaderPattern(AUTOBIND_META_PATTERN, autometa),
                            HeaderPattern(BUILDABLE_PATTERN, autobindable))

    if bindable_paths:
        acg.matchHeaderPatterns(bindable_paths.split(path.pathsep), HeaderPattern(BUILDABLE_PATTERN, autobindable))

    if output_cmakelist:
        return acg.write_to_file(_just_print_flag and output_cmakelist, gen_cmakelist_source(params, paths, output_dir, output_file or 'stdout', results))

    head_src = gen_header_source(output_file or 'stdout', output_dir, output_file, results, namespaces)
    cpp_src = gen_body_source(output_file or 'stdout', output_dir, output_file, namespaces, modulename, results, bindables)
    if output_file or _just_print_flag is None:
        acg.write_to_file(_just_print_flag and output_file + '.h', head_src)
        acg.write_to_file(_just_print_flag and output_file + '.cpp', cpp_src)


if __name__ == '__main__':
    opts, paths = getopt.getopt(sys.argv[1:], 'b:c:p:n:o:l:m:d:j')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in ['p', 'm']) or len(paths) == 0:
        print('Usage: %s -p namespace -m modulename [-c cmakefile] [-o output_file] [-l library_path] dir_paths...' % sys.argv[0])
        sys.exit(0)

    library_path = params['l'] if 'l' in params else None
    if library_path and library_path not in sys.path:
        sys.path.append(library_path)

    import acg
    from acg import HeaderPattern

    if 'j' in params:
        _just_print_flag = None

    main(params, paths)
