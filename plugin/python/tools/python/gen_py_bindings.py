#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import sys
import getopt
from os import path
import re

LOADER_TEMPLATE = '''template<typename T> Box PyArk${classname}_${methodname}Function(PyArkType::Instance& inst, const String& id, const sp<Scope>& args) {
    const sp<T> bean = inst.unpack<${classname}>()->${methodname}<T>(id, args);
    return bean.pack();
}'''
ANNOTATION_PATTERN = r'(?:\s*(?://)?\s*\[\[[^]]+\]\])*'
AUTOBIND_CONTANT_PATTERN = re.compile(r'\[\[script::bindings::constant\(([\w\d_]+)\s*=\s*([^;\r\n]*)\)\]\]')
AUTOBIND_ENUMERATION_PATTERN = re.compile(r'\[\[script::bindings::enumeration\]\]\s*enum\s+(\w+)\s*\{([^}]+)};')
AUTOBIND_PROPERTY_PATTERN = re.compile(r'\[\[script::bindings::property\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_LOADER_PATTERN = re.compile(r'\[\[script::bindings::loader\]\]\s+template<typename T>\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;{]*\{')
AUTOBIND_METHOD_PATTERN = re.compile(r'\[\[script::bindings::(auto|classmethod)\]\]\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;\r\n]*;')
AUTOBIND_ANNOTATION_PATTERN = re.compile(r'\[\[script::bindings::(auto|container)\]\]%s\s+class\s+([^{\r\n]+)\s*{' % ANNOTATION_PATTERN)
AUTOBIND_CLASS_PATTERN = re.compile(r'\[\[script::bindings::class\(([^)]+)\)\]\]\s+class\s+')
AUTOBIND_META_PATTERN = re.compile(r'\[\[script::bindings::meta\(([^)]+)\([^)]*\)\)\]\]')

BUILDABLE_PATTERN = re.compile(r'\[\[plugin::(?:builder|resource-loader)[^\]]*\]\]\s+class\s+[\w\d_]+\s*:\s*public\s+Builder<([^{]+)>\s*\{')

CLASS_DELIMITER = '\n//%s\n' % ('-' * 120)
TYPE_DEFINED_SP = ('document', 'element', 'attribute', 'bitmap')
TYPE_DEFINED_OBJ = ('V2', 'V3', 'V4')

cwd = os.getcwd()


def make_abs_path(p):
    return p if path.isabs(p) else path.join(cwd, p)


def gen_cmakelist_source(arguments, paths, output_dir, output_file, results):
    generated_files = [make_abs_path(path.join(output_dir, output_file + i)) for i in ('.h', '.cpp')]
    dependencies = set()
    for k, genclass in results.items():
        generated_files.append(make_abs_path(path.join(output_dir, genclass.py_type_name + ".h")))
        generated_files.append(make_abs_path(path.join(output_dir, genclass.py_type_name + ".cpp")))
        dependencies.add(make_abs_path(genclass.filename))
    return acg.format('''set(LOCAL_PY_TYPE_SOURCE_LIST
    %s
)
set(LOCAL_PY_TYPE_DEPENDENCY_LIST
    %s
)
add_custom_command(OUTPUT #{LOCAL_PY_TYPE_SOURCE_LIST}
    COMMAND python ${script_path} ${arguments} ${paths}
    DEPENDS #{LOCAL_PY_TYPE_DEPENDENCY_LIST} ${script_path}
    WORKING_DIRECTORY ${working_dir})
list(APPEND LOCAL_GENERATED_SRC_LIST #{LOCAL_PY_TYPE_SOURCE_LIST})
''', script_path=make_abs_path(sys.argv[0]),
                      working_dir=os.getcwd(),
                      arguments=' '.join('-%s %s' % (j, k) for j, k in arguments.items() if j != 'c'),
                      paths=' '.join(paths)).replace('#', '$') % ('\n    '.join(sorted(generated_files)),
                                                                  '\n    '.join(sorted(list(dependencies))))


def gen_header_source(filename, output_dir, results, namespaces):
    filedir, name = path.split(filename)
    declares = ['\nvoid __init_%s__(PyObject* module);' % name]
    includes = []
    for k, genclass in results.items():
        if output_dir is None:
            declares.append(CLASS_DELIMITER)
            gen_class_header_source(genclass, declares)
        else:
            local_declares = []
            includes.append('#include "%s.h"' % genclass.py_type_name)
            gen_class_header_source(genclass, local_declares)
            hfilepath = path.join(output_dir, genclass.py_type_name + '.h')
            with open(hfilepath, 'wt') as fp:
                fp.write(gen_py_binding_h(hfilepath, namespaces, [], local_declares))
    return gen_py_binding_h(filename, namespaces, includes, declares)


def gen_class_header_source(genclass, declares):
    method_declarations = [i.gen_declaration() for i in genclass.methods]
    s = '\n    '.join(i for i in method_declarations if i)
    declares.append(acg.format('''class PyArk${class_name}Type : public ark::plugin::python::PyArkType {
public:
    PyArk${class_name}Type(const String& name, const String& doc, long flags);

    ${method_declares}
};''', method_declares=s, class_name=genclass.classname))


def gen_py_binding_h(filename, namespaces, includes, declares):
    header_macro = re.sub(r'[^\w\d]', '_', filename).upper()
    return acg.format('''#ifndef PY_${header_macro}_GEN
#define PY_${header_macro}_GEN

${includes}

#include "python/extension/py_ark_type.h"

${0}

#endif''', acg.wrapByNamespaces(namespaces, '\n'.join(declares)), header_macro=header_macro,
                      includes='\n'.join(includes))


def gen_body_source(filename, output_dir, namespaces, modulename, results, buildables):
    filedir, name = path.split(filename)
    includes = []
    lines = []
    for k, genclass in results.items():
        if output_dir is None:
            lines.append(CLASS_DELIMITER)
            gen_class_body_source(genclass, includes, lines, buildables)
        else:
            classincludes = []
            classlines = []
            includes.append('#include "%s"' % genclass.filename)
            gen_class_body_source(genclass, classincludes, classlines, buildables)
            with open(path.join(output_dir, genclass.py_type_name + '.cpp'), 'wt') as fp:
                fp.write(gen_py_binding_cpp(genclass.py_type_name, namespaces, classincludes, classlines))

    add_types = '\n    '.join('pi->pyModuleAddType<PyArk%sType, %s>(module, "%s", "%s", Py_TPFLAGS_DEFAULT%s);' % (i, i, modulename, j.type_name, '|Py_TPFLAGS_HAVE_GC' if j.is_container else '') for i, j in results.items())
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
        method_defs = [i.gen_py_method_def(genclass.classname) for i in genclass.methods]
        methoddeclare = ',\n    '.join(i for i in method_defs if i)
        if methoddeclare:
            lines.append('\nstatic PyMethodDef PyArk%s_methods[] = {\n    %s,\n    {NULL}\n};' % (
                genclass.classname, methoddeclare))
            tp_method_lines.append('pyTypeObject->tp_methods = PyArk%s_methods;' % genclass.classname)

        property_defs = genclass.gen_property_defs()
        if property_defs:
            propertydeclare = '{"%s", %s, %s, "%s"}'
            methoddeclare = ',\n    '.join(propertydeclare % tuple(i) for i in property_defs)
            lines.append('\nstatic PyGetSetDef PyArk%s_getseters[] = {\n    %s,\n    {NULL}\n};' % (
            genclass.classname, methoddeclare))
            tp_method_lines.append('pyTypeObject->tp_getset = PyArk%s_getseters;' % genclass.classname)

        for i in genclass.loader_methods():
            lines.append(acg.format(LOADER_TEMPLATE, classname=genclass.classname, methodname=i.name))
            tp_method_lines.append('{')
            tp_method_lines.append('    std::map<TypeId, LoaderFunction>& loader = ensureLoader("%s");' % i.name)
            for j in buildables:
                if j.find('::') == -1:
                    tp_method_lines.append('    ' + i.gen_loader_statement(genclass.classname, j, i.name))
            tp_method_lines.append('}')

    genclass.gen_py_type_constructor_codes(tp_method_lines)
    if genclass.constants:
        tp_method_lines.extend('_constants["%s"] = %s;' % (i, j) for i, j in genclass.constants.items())
    constructor = acg.format('''PyArk${class_name}Type::PyArk${class_name}Type(const String& name, const String& doc, long flags)
    : PyArkType(name, doc, flags)
{${tp_methods}
}''', class_name=genclass.classname, tp_methods='\n    ' + '\n    '.join(tp_method_lines) if tp_method_lines else '')
    methoddefinition = acg.format('''%s PyArk${class_name}Type::%s(%s)
{
    %s
}''', class_name=genclass.classname)
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
#include "python/extension/wrapper.h"

%s

using namespace ark::plugin::python;

''' % (name, '\n'.join(includes))) + acg.wrapByNamespaces(namespaces, '\n'.join(lines))


class GenArgumentMeta:
    def __init__(self, typename, castsig, parsetuplesig):
        self._typename = typename
        self._castsig = castsig
        self._parse_tuple_sig = parsetuplesig


class GenConverter:
    def __init__(self, check_func, cast_func):
        self._check_func = check_func
        self._cast_func = cast_func

    def check(self, var):
        return self._check_func % var

    def cast(self, var):
        return self._cast_func % var


class GenArgument:
    def __init__(self, accept_type, default_value, meta):
        self._accept_type = accept_type
        self._default_value = default_value
        self._meta = meta

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
    def parse_format_units(self):
        s = self._meta._parse_tuple_sig
        return s if self._default_value is None else '|' + s

    def gen_declare(self, objname, argname):
        typename = self.meta._castsig
        if self.meta._typename == self.meta._castsig:
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
            return self._gen_var_declare(m, objname, 'toSharedPtr', m[0].upper() + m[1:], argname)
        if m != self._accept_type:
            return self._gen_var_declare('sp<%s>' % m, objname, 'toSharedPtr', m, argname)
        return self._gen_var_declare(typename, objname, 'toType', typename, argname)

    def _gen_var_declare(self, typename, varname, funcname, functype, argname):
        if self._default_value and self._accept_type.startswith('sp<'):
            return 'const %s %s = %s ? PythonInterpreter::instance()->%s<%s>(%s) : %s;' % (
                typename, varname, argname, funcname, functype, argname, self._default_value)
        return 'const %s %s = PythonInterpreter::instance()->%s<%s>(%s);' % (typename, varname, funcname, functype, argname)


ARK_PY_ARGUMENTS = (
    (r'const\s+String\s*&?', GenArgumentMeta('const char*', 'const char*', 's')),
    (r'const\s+sp<Scope>\s*&', GenArgumentMeta('PyObject*', 'sp<Scope>', '')),
    (r'const\s+std::wstring\s*&', GenArgumentMeta('PyObject*', 'std::wstring', 'O')),
    (r'const\s+Box\s*&', GenArgumentMeta('PyObject*', 'Box', 'O')),
    (r'const\s+sp<([^>]+|[\w\d_]+<[\w\d_]+>)>\s*&', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'const\s+(document|element|attribute)\s*&', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (r'(?:const\s+)?([^>]+|[\w\d_]+<[\w\d_]+>)\s*&', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'(uint32_t|unsigned int|uint8_t)', GenArgumentMeta('uint32_t', 'uint32_t', 'I')),
    (r'(int32_t|int|int8_t)', GenArgumentMeta('int32_t', 'int32_t', 'i')),
    (r'float', GenArgumentMeta('float', 'float', 'f')),
    (r'bool', GenArgumentMeta('int32_t', 'int32_t', 'p')),
    (r'[^:]+::.+', GenArgumentMeta('uint32_t', 'uint32_t', 'I')),
    (r'TypeId', GenArgumentMeta('PyObject*', 'TypeId', 'O')),
)

ARK_PY_ARGUMENT_CHECKERS = {
    'i': GenConverter('PyLong_Check(%s)', 'PyLong_AsLong(%s)'),
    'f': GenConverter('PyFloat_Check(%s)', '﻿PyFloat_AsDouble(%s)'),
    'O': GenConverter('PyLong_Check(%s)', 'PyLong_AsLong(%s)'),
}


def parse_method_arguments(arguments):
    declares = {}
    args = []
    for i, j in enumerate(arguments):
        m = None
        for k, l in ARK_PY_ARGUMENTS:
            m = re.match(k, j)
            typename = l._typename
            if m:
                default_value = None
                pos = j.find('=')
                if pos != -1 and not j.startswith('const sp<Scope>'):
                    default_value = j[pos + 1:].strip()
                    if default_value.endswith('('):
                        default_value += ')'
                if typename in declares:
                    declares[typename] = '%s, %sarg%d' % (declares[typename], '*' if typename.endswith('*') else '', i)
                else:
                    declares[typename] = '%s arg%d' % (typename, i)
                if default_value is not None:
                    declares[typename] += ' = %s' % ('0' if typename in ('uint32_t', 'int32_t', 'float') else 'nullptr')
                elif not l._parse_tuple_sig:
                    declares[typename] += ' = kws'
                accept_type = acg.format(l._castsig, *m.groups())
                args.append(GenArgument(accept_type, default_value, l))
                break
        if not m:
            print('Undefined method argument: "%s"' % arguments[i])
            sys.exit(-1)
    return [i + ';' for i in declares.values()], args


def gen_method_call_arg(i, arg, argtype):
    pos = arg.find('=')
    if pos != -1:
        arg = arg[0:pos]
    targettype = ' '.join(arg.split()[:-1])
    equals = acg.typeCompare(targettype, argtype)
    if targettype == 'bool':
        return 'obj%d != 0' % i
    return 'obj%d' % i if equals or '<' in argtype else 'static_cast<%s>(obj%d)' % (targettype, i)


class GenMethod(object):
    def __init__(self, name, args, return_type):
        self._name = name
        self._return_type = return_type
        self._arguments = args
        self._has_keyvalue_arguments = self._arguments and acg.typeCompare('sp<Scope>', self._arguments[-1])
        self._flags = ('METH_VARARGS|METH_KEYWORDS' if self._has_keyvalue_arguments else 'METH_VARARGS')
        self._arg_declares = [i.split('=')[0].strip() for i in self._arguments]

    def gen_py_method_def(self, classname):
        return None

    def gen_py_return(self):
        return 'PyObject*'

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* args' + (', PyObject* kws' if self._has_keyvalue_arguments else '')

    @property
    def name(self):
        return self._name

    @property
    def _err_return_value(self):
        return 'Py_RETURN_NONE'

    def _gen_call_statement(self, genclass, argnames):
        return 'unpacked->%s(%s);' % (self._name, argnames)

    @staticmethod
    def _gen_return_statement(return_type):
        m = acg.getSharedPtrType(return_type)
        if m in ('String', 'std::wstring'):
            fromcall = 'template fromType<%s>' % m
        elif m == 'PyObject*':
            return 'return ret;'
        else:
            fromcall = 'toPyObject'
        return 'return PythonInterpreter::instance()->%s(ret);' % fromcall

    def _gen_declare_locals_code(self, lines, declares, args, argdeclare):
        parse_format = ''.join(i.parse_format_units for i in args)
        if parse_format.count('|') > 1:
            ts = parse_format.split('|')
            if any(len(i) > 1 for i in ts):
                print('Illegal default arguments: %s', parse_format)
                sys.exit(-1)
            parse_format = ts[0] + '|' + ''.join(ts[1:])
        parsestatement = '''%s
    if(!PyArg_ParseTuple(args, "%s", %s))
        %s;
''' % ('\n    '.join(declares), parse_format, ', '.join('&arg%d' % i for i, j in enumerate(args) if j.meta._parse_tuple_sig), self._err_return_value)
        lines.append(parsestatement)
        lines.append('\n    '.join(argdeclare))

    def _need_unpack_statement(self):
        return True

    def gen_py_type_constructor_codes(self, lines, classname):
        pass

    def gen_declaration(self):
        return 'static %s %s(%s);' % (self.gen_py_return(), self._name, self.gen_py_arguments())

    def gen_definition(self, genclass):
        lines = []
        argtypes = []
        if len(self._arguments) > 0:
            declares, args = parse_method_arguments(self._arguments)
            argdeclare = [j.gen_declare('obj%d' % i, 'arg%d' % i) for i, j in enumerate(args)]
            argtypes = [i.split()[0] for i in argdeclare]
            self._gen_declare_locals_code(lines, declares, args, argdeclare)
        if self._need_unpack_statement():
            lines.append(acg.format('const sp<${class_name}>& unpacked = self->unpack<${class_name}>();', class_name=genclass._classname))
        r = acg.stripKeywords(self._return_type, ['virtual', 'const', '&'])
        argnames = ', '.join(gen_method_call_arg(i, j, argtypes[i]) for i, j in enumerate(self._arguments))
        callstatement = self._gen_call_statement(genclass, argnames)
        pyret = 'return 0;' if self.gen_py_return() == 'int' else 'Py_RETURN_NONE;'
        if r != 'void' and r:
            callstatement = '%s ret = %s' % (acg.stripKeywords(self._return_type, ['virtual']), callstatement)
            pyret = self._gen_return_statement(r)
        lines.append(callstatement)
        lines.append(pyret)
        return '\n    '.join(lines)

    @staticmethod
    def split(repl):
        name = repl[0].split()[-1]
        rs = repl[0].split()[:-1]
        return_type = ' '.join(i for i in rs if i not in('static', 'ARK_API'))
        args = [i.strip() for i in repl[1].split(',') if i]
        return name, args, return_type, 'static' in rs


class GenConstructorMethod(GenMethod):
    def __init__(self, name, args):
        GenMethod.__init__(self, '__init__', args, '')
        self._funcname = name

    def _gen_call_statement(self, genclass, argnames):
        return 'self->box = new Box(sp<%s>::make(%s).pack());' % (self._funcname, argnames)

    def gen_py_return(self):
        return 'int'

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* args, PyObject* /*kws*/'

    @property
    def _err_return_value(self):
        return 'return -1'

    def _need_unpack_statement(self):
        return False

    def gen_py_type_constructor_codes(self, lines, classname):
        lines.append('pyTypeObject->tp_init = reinterpret_cast<initproc>(PyArk%sType::__init__);' % classname)


class GenMetaMethod(GenMethod):
    def __init__(self, name):
        GenMethod.__init__(self, name, [], '')

    def gen_declaration(self):
        return None

    def gen_definition(self, classname):
        return None

    def gen_py_method_def(self, classname):
        return '{"%s", (PyCFunction) PyArkMetaType<%s>::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), classname, self._name, self._flags)


class GenPropertyMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)
        self._is_setter = name.startswith('set')
        self._property_name = name[3].lower() + name[4:] if self._is_setter else name

    def gen_py_return(self):
        return 'int' if self._is_setter else "PyObject*"

    def _gen_declare_locals_code(self, lines, declares, args, argdeclare):
        ga = GenArgument(args[0].accept_type, args[0].default_value, GenArgumentMeta('PyObject*', args[0].accept_type, 'O'))
        lines.append(ga.gen_declare('obj0', 'value'))

    def gen_py_arguments(self):
        if self._is_setter:
            return 'Instance* self, PyObject* value, void* /*closure*/'
        return 'Instance* self, PyObject* args'

    def gen_py_getset_def(self, properties, classname):
        property_def = self._ensure_property_def(properties)
        if self._is_setter:
            property_def[2] = '(setter) PyArk%sType::%s' % (classname, self._name)
        else:
            property_def[1] = '(getter) PyArk%sType::%s' % (classname, self._name)

    def _gen_call_statement(self, genclass, argnames):
        lines = ['unpacked->%s(%s);' % (self._name, argnames)]
        if self._is_setter and genclass.is_container and acg.typeCompare(self._arguments[0], 'Box'):
            lines.append('self->setContainerObject(obj0);')
        return '\n    '.join(lines)

    def _ensure_property_def(self, properties):
        snake_property_name = acg.camel_case_to_snake_case(self._property_name)
        for i in properties:
            if i[0] == snake_property_name:
                return i
        property_def = [snake_property_name, 'nullptr', 'nullptr', self._property_name]
        properties.append(property_def)
        return property_def


class GenLoaderMethod(GenMethod):
    def __init__(self, name, args):
        GenMethod.__init__(self, name, ['TypeId typeId'] + args, 'PyObject*')

    def gen_py_method_def(self, classname):
        return '{"%s", (PyCFunction) PyArk%sType::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), classname, self._name, self._flags)

    def _gen_call_statement(self, genclass, argnames):
        return 'PythonInterpreter::instance()->ensurePyArkType(reinterpret_cast<PyObject*>(self))->load(*self, "%s", %s);' % (self._name, argnames)

    def _need_unpack_statement(self):
        return False

    def gen_loader_statement(self, classname, beanname, methodname):
        return acg.format('loader[Type<${beanname}>::id()] = PyArk${classname}_${methodname}Function<${beanname}>;', classname=classname, beanname=beanname, methodname=methodname)


class GenMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_method_def(self, classname):
        return '{"%s", (PyCFunction) PyArk%sType::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), classname, self._name, self._flags)


class GenStaticMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_method_def(self, classname):
        return '{"%s", (PyCFunction) PyArk%sType::%s, %s|METH_STATIC, nullptr}' % (acg.camel_case_to_snake_case(self._name), classname, self._name, self._flags)

    def _need_unpack_statement(self):
        return False

    def _gen_call_statement(self, genclass, argnames):
        return '%s::%s(%s);' % (genclass.classname, self._name, argnames)


class GenStaticMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args[1:], return_type)

    def gen_py_method_def(self, classname):
        return '{"%s", (PyCFunction) PyArk%sType::%s, %s, nullptr}' % (acg.camel_case_to_snake_case(self._name), classname, self._name, self._flags)

    def _gen_call_statement(self, genclass, argnames):
        return '%s::%s(unpacked%s);' % (genclass.classname, self._name, argnames if not argnames else ', ' + argnames)


class GenClass(object):
    def __init__(self, filename, class_name, is_container=False):
        self._py_type_name = 'py_ark_' + acg.camel_case_to_snake_case(class_name)
        self._filename = filename
        self._classname = class_name
        self._type_name = class_name
        self._is_container = is_container
        self._methods = []
        self._constants = {}

    @property
    def classname(self):
        return self._classname

    @property
    def type_name(self):
        return self._type_name

    @type_name.setter
    def type_name(self, v):
        self._type_name = v

    @property
    def py_type_name(self):
        return self._py_type_name

    @property
    def methods(self):
        return self._methods

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
        self._methods.append(method)

    def add_constant(self, name, value):
        self._constants[name] = value

    def has_methods(self):
        return len(self._methods) > 0

    def property_methods(self):
        return [i for i in self._methods if type(i) is GenPropertyMethod]

    def loader_methods(self):
        return [i for i in self._methods if type(i) is GenLoaderMethod]

    def gen_py_type_constructor_codes(self, lines):
        for i in self._methods:
            i.gen_py_type_constructor_codes(lines, self._classname)

    def gen_property_defs(self):
        property_defs = []
        for i in self.property_methods():
            i.gen_py_getset_def(property_defs, self._classname)
        return property_defs


def get_result_class(results, filename, classname):
    genclass = results[classname] if classname in results else GenClass(filename, classname)
    if classname not in results:
        results[classname] = genclass
    return genclass


def main(params, paths):
    results = {}
    bindables = {'Object'}

    namespaces = params['p'].replace('::', ':').split(':')
    modulename = params['m']
    output_file = params['o'] if 'o' in params else None
    output_dir = params['d'] if 'd' in params else None
    output_cmakelist = params['c'] if 'c' in params else None

    def automethod(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        method_modifier = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        if is_static and method_modifier == 'auto':
            genmethod = GenStaticMethod(name, args, return_type)
        elif genclass.classname == name:
            genmethod = GenConstructorMethod(name, args)
        elif is_static and method_modifier == 'classmethod':
            genmethod = GenStaticMemberMethod(name, args, return_type)
        else:
            genmethod = GenMemberMethod(name, args, return_type)
        genclass.add_method(genmethod)

    def autoloader(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenLoaderMethod(name, args))

    def autoproperty(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenPropertyMethod(name, args, return_type))

    def autoannotation(filename, content, m, x):
        names = [i for i in x[1].split(':', 2)[0].split() if i not in ('ARK_API', 'final')]
        if len(names) == 1:
            results[names[0]] = GenClass(filename, names[0], x[0].strip() == 'container')

    def autoclass(filename, content, main_class, x):
        genclass = get_result_class(results, filename, main_class)
        genclass.type_name = x.strip('"')

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
            varname = i.strip().split('=')[0]
            if varname:
                genclass.add_constant(varname, '%s::%s' % (main_class, varname))

    acg.matchHeaderPatterns(paths,
                            {'pattern': AUTOBIND_ANNOTATION_PATTERN, 'callback': autoannotation},
                            {'pattern': AUTOBIND_CLASS_PATTERN, 'callback': autoclass},
                            {'pattern': AUTOBIND_METHOD_PATTERN, 'callback': automethod},
                            {'pattern': AUTOBIND_LOADER_PATTERN, 'callback': autoloader},
                            {'pattern': AUTOBIND_PROPERTY_PATTERN, 'callback': autoproperty},
                            {'pattern': AUTOBIND_CONTANT_PATTERN, 'callback': autoconstant},
                            {'pattern': AUTOBIND_ENUMERATION_PATTERN, 'callback': autoenumeration},
                            {'pattern': AUTOBIND_META_PATTERN, 'callback': autometa},
                            {'pattern': BUILDABLE_PATTERN, 'callback': autobindable}
                            )

    if output_cmakelist:
        return acg.write_to_file(output_cmakelist if output_cmakelist != 'stdout' else None, gen_cmakelist_source(params, paths, output_dir, output_file or 'stdout', results))
    acg.write_to_file(output_file and output_file + '.h', gen_header_source(output_file or 'stdout', output_dir, results, namespaces))
    acg.write_to_file(output_file and output_file + '.cpp',
                      gen_body_source(output_file or 'stdout', output_dir, namespaces, modulename, results, bindables))


if __name__ == '__main__':
    opts, paths = getopt.getopt(sys.argv[1:], 'c:p:n:o:l:m:d:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in ['p', 'm']) or len(paths) == 0:
        print('Usage: %s -p namespace -m modulename [-c cmakefile] [-o output_file] [-l library_path] dir_paths...' % sys.argv[0])
        sys.exit(0)

    library_path = params['l'] if 'l' in params else None
    if library_path and library_path not in sys.path:
        sys.path.append(library_path)
    import acg

    main(params, paths)
