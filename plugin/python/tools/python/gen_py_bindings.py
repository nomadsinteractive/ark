#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import re
import sys
from os import path
from typing import Optional

from gen_core import get_param_and_paths, import_acg, INDENT, GenArgumentMeta, GenArgument, create_overloaded_method_type, get_params
from gen_method import GenMethod, GenGetPropMethod, GenSetPropMethod, GenMappingMethod, gen_operator_defs, gen_as_mapping_defs, GenOperatorMethod, \
    GenSequenceMethod, gen_as_sequence_defs

LOADER_TEMPLATE = '''template<typename T> static Box PyArk${classname}_${methodname}Function(PyArkType::Instance& inst, const String& id, const Scope& args) {
    const sp<T> bean = inst.unpack<${classname}>()->${methodname}<T>(id, args);
    return Box(bean);
}'''

RICH_COMPARE_OPS = {
    '<': 'Py_LT',
    '<=': 'Py_LE',
    '==': 'Py_EQ',
    '!=': 'Py_NE',
    '>': 'Py_GT',
    '>=': 'Py_GE'
}

ANNOTATION_PATTERN = r'(?:\s*(?://)?\s*\[\[[^]]+]])*'
METHOD_PATTERN = r'([^(\r\n]+)\(([^\r\n]*)\)[^;\r\n]*;'
DECLARATION_PATTERN = r'(static\s+)?(const\s+)?([\w\s]+);'

AUTOBIND_ENUMERATION_PATTERN = re.compile(r'\[\[script::bindings::enumeration]]\s*enum\s+(\w+)\s*{([^}]+)};')
AUTOBIND_PROPERTY_PATTERN = re.compile(r'\[\[script::bindings::property]]\s+%s' % METHOD_PATTERN)
AUTOBIND_GETPROP_PATTERN = re.compile(r'\[\[script::bindings::getprop]]\s+%s' % METHOD_PATTERN)
AUTOBIND_SETPROP_PATTERN = re.compile(r'\[\[script::bindings::setprop]]\s+%s' % METHOD_PATTERN)
AUTOBIND_LOADER_PATTERN = re.compile(r'\[\[script::bindings::loader]]\s+template<typename T>\s+([^(\r\n]+)\(([^)\r\n]*)\)[^;{]*{')
AUTOBIND_METHOD_PATTERN = re.compile(r'\[\[script::bindings::(auto|classmethod|constructor)]]\s+%s' % METHOD_PATTERN)
AUTOBIND_AS_MAPPING_PATTERN = re.compile(r'\[\[script::bindings::map\(([^)]+)\)]]\s+%s' % METHOD_PATTERN)
AUTOBIND_AS_SEQUENCE_PATTERN = re.compile(r'\[\[script::bindings::seq\(([^)]+)\)]]\s+%s' % METHOD_PATTERN)
AUTOBIND_OPERATOR_PATTERN = re.compile(r'\[\[script::bindings::operator\(([^)]+)\)]]\s+%s' % METHOD_PATTERN)
AUTOBIND_CLASS_PATTERN = re.compile(r'\[\[script::bindings::(class|name)\(([^)]+)\)]]')
AUTOBIND_EXTENDS_PATTERN = re.compile(r'\[\[script::bindings::extends\((\w+)\)]]')
AUTOBIND_TYPEDEF_PATTERN = re.compile(r'\[\[script::bindings::auto]]\s+typedef\s+\w[\w<>\s]+\s+(\w+);')
AUTOBIND_ANNOTATION_PATTERN = re.compile(r'\[\[script::bindings::(auto|container|holder)]]%s\s+class\s+([^{\r\n]+)\s*{' % ANNOTATION_PATTERN)

BUILDABLE_PATTERN = re.compile(r'\[\[plugin::(?:builder|resource-loader)[^]]*]]\s+class\s+\w+\s*(?:final)?\s*:\s*public\s+Builder<([^{]+)>\s*{')

CLASS_DELIMITER = '\n//%s\n' % ('-' * 120)
ARK_CORE_BUILDABLES = {'AudioPlayer'}

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
    script_path = make_abs_path(sys.argv[0])
    script_dir = path.dirname(script_path)
    script_deps = ' '.join([f'{script_dir}/{i}' for i in ('gen_core.py', 'gen_method.py')])
    return acg.format('''macro(ark_add_py_binding SRC_PATH SRC_ABS_PATH)
    add_custom_command(OUTPUT #{ARGN}
        COMMAND #{Python_EXECUTABLE} ${script_path} $<$<CONFIG:RELEASE>:-t> -b "${bindable_paths}" ${arguments_without_o} #{SRC_PATH}
        DEPENDS #{SRC_ABS_PATH} ${script_path} ${script_deps}
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
        COMMAND #{Python_EXECUTABLE} ${script_path} ${arguments_without_d} ${paths}
        DEPENDS #{LOCAL_PY_TYPE_DEPENDENCY_LIST} ${script_path} ${script_deps}
        WORKING_DIRECTORY ${working_dir})
list(APPEND LOCAL_GENERATED_SRC_LIST ${output_files})

''', script_path=script_path, script_deps=script_deps,
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
    declares = [f'\nvoid __init_{name.replace("-", "_")}__(PyObject* module);']
    includes = []
    for k, genclass in results.items():
        if output_dir is None:
            declares.append(CLASS_DELIMITER)
            gen_class_header_source(genclass, declares)
        else:
            includes.append(f'#include "{genclass.py_src_name}.h"')
            if output_file is None:
                local_declares = []
                gen_class_header_source(genclass, local_declares)
                hfilepath = path.join(output_dir, genclass.py_src_name + '.h')
                acg.write_to_file(_just_print_flag and hfilepath, gen_py_binding_h(hfilepath, namespaces, [], local_declares))
    return gen_py_binding_h(filename, namespaces, includes, declares)


def gen_class_header_source(genclass, declares):
    method_declarations = sum(filter(None, [i.gen_declaration() for i in genclass.methods]), [])
    s = '\n    '.join(method_declarations)
    declares.append(acg.format('''class ${py_class_name} : public ark::plugin::python::PyArkType {
public:
    ${py_class_name}(const String& name, const String& doc, PyTypeObject* base, unsigned long flags);

    ${method_declares}
};''', method_declares=s, py_class_name=genclass.py_class_name))


def gen_py_binding_h(filename, namespaces, includes, declares):
    header_macro = re.sub(r'\W', '_', filename).upper()
    return acg.format('''#pragma once

${includes}

#include "python/extension/py_ark_type.h"
#include "python/extension/py_bridge.h"
#include "python/extension/py_cast.h"

${0}
''', acg.wrap_by_namespaces(namespaces, '\n'.join(declares)), header_macro=header_macro,
                      includes='\n'.join(includes))


def gen_module_type_declarations(modulename, results):
    line_pattern = 'pi.pyModuleAddType<%s, %s>(module, "%s", "%s", %s, Py_TPFLAGS_DEFAULT%s)'
    genclasses = list(results.values())
    class_names = set(i.binding_classname for i in genclasses)
    class_declared = set()
    declarations = []

    while genclasses:
        i = genclasses[0]
        genclasses = genclasses[1:]
        if not i.base_classname or i.base_classname in class_declared or i.base_classname not in class_names:
            base_type = 'pi.getPyArkType<%s>()->getPyTypeObject()' % i.base_classname if i.base_classname else 'nullptr'
            declarations.append(line_pattern % (i.py_class_name, i.binding_classname, modulename, i.name, base_type,
                                                '|Py_TPFLAGS_HAVE_GC' if i.is_container else ''))
            class_declared.add(i.binding_classname)
        else:
            assert len(genclasses) != 0
            genclasses.append(i)

    return declarations


def gen_body_source(filename, output_dir, output_file, namespaces, modulename, results, buildables, gen_bindingcpp):
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

    if not gen_bindingcpp:
        return None

    add_types = '\n    '.join([f'moduletypes.push_back({i});' for i in gen_module_type_declarations(modulename, results)])
    lines.append('\n' + '''void __init_%s__(PyObject* module)
{
    std::vector<PyArkType*> moduletypes;
    ark::plugin::python::PythonInterpreter& pi = ark::plugin::python::PythonInterpreter::instance();
    %s
    for(PyArkType* i : moduletypes)
        i->onReady();
}''' % (name.replace('-', '_'), add_types))

    return gen_py_binding_cpp(name, namespaces, includes, lines)


def gen_constructor_definition_source(py_class_name: str, tp_name: str, definitions: list[str], constructor_section: list[str], declaration_section: list[str],
                                      cast_type: Optional[str] = None):
    if definitions:
        constructor_section.append('')
        constructor_section.extend(text_align(text_align(definitions, '/*'), '*/'))
        value = f'&{py_class_name}_tp_{tp_name}'
        if cast_type:
            value = f'reinterpret_cast<{cast_type}>({value})'
        declaration_section.append(f'pyTypeObject->tp_{tp_name} = {value};')


def gen_class_body_source(genclass, includes, lines, buildables):
    tp_method_lines = []
    includes.append(f'#include "{genclass.filename}"')
    if genclass.has_methods():
        tp_method_lines.append('PyTypeObject* pyTypeObject = getPyTypeObject();')
        method_defs = [i.gen_py_method_def(genclass) for i in genclass.methods]
        methoddeclare = ',\n    '.join(i for i in method_defs if i)
        if methoddeclare:
            lines.append('\nstatic PyMethodDef %s_methods[] = {\n    %s,\n    {nullptr}\n};' % (genclass.py_class_name, methoddeclare))
            tp_method_lines.append(f'pyTypeObject->tp_methods = {genclass.py_class_name}_methods;')

        getprop_methods = genclass.getprop_methods()
        if getprop_methods:
            if len(getprop_methods) > 1:
                print("WARNING: at least two getprop methods defined in %s, which is probably not right", genclass.classname)
            tp_method_lines.append('pyTypeObject->tp_getattro = (getattrofunc) %s;' % getprop_methods[0].name)

        setprop_methods = genclass.setprop_methods()
        if setprop_methods:
            if len(setprop_methods) > 1:
                print("WARNING: at least two setprop methods defined in %s, which is probably not right", genclass.classname)
            tp_method_lines.append('pyTypeObject->tp_setattro = (setattrofunc) %s;' % setprop_methods[0].name)

        as_sequence_defs = gen_as_sequence_defs(genclass)
        gen_constructor_definition_source(genclass.py_class_name, 'as_sequence', as_sequence_defs, lines, tp_method_lines)

        as_mapping_defs = gen_as_mapping_defs(genclass)
        gen_constructor_definition_source(genclass.py_class_name, 'as_mapping', as_mapping_defs, lines, tp_method_lines)

        operator_defs = gen_operator_defs(genclass)
        gen_constructor_definition_source(genclass.py_class_name, 'as_number', operator_defs, lines, tp_method_lines)

        rich_compare_defs = genclass.gen_rich_compare_defs()
        gen_constructor_definition_source(genclass.py_class_name, 'richcompare', rich_compare_defs, lines, tp_method_lines, 'richcmpfunc')

        property_defs = genclass.gen_property_defs()
        if property_defs:
            propertydeclare = '{"%s", %s, %s, "%s", nullptr}'
            methoddeclare = ',\n    '.join(propertydeclare % tuple(i) for i in property_defs)
            lines.append('\nstatic PyGetSetDef %s_getseters[] = {\n    %s,\n    {%s}\n};'
                         % (genclass.py_class_name, methoddeclare, ', '.join(['nullptr'] * 5)))
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

    tp_method_lines.extend(f'_enum_constants["{k}"] = Box({v});' for k, v in genclass.enum_constants.items())

    constructor = acg.format('''${py_class_name}::${py_class_name}(const String& name, const String& doc, PyTypeObject* base, unsigned long flags)
    : PyArkType(name, doc, base, flags)
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

%s

using namespace ark::plugin::python;

''' % (name, '\n'.join(includes))) + acg.wrap_by_namespaces(namespaces, '\n'.join(lines))


class GenConstructorMethod(GenMethod):
    def __init__(self, name, args, is_static):
        GenMethod.__init__(self, '__init__', args, '')
        self._funcname = name
        self._is_static = is_static

    def _gen_calling_statement(self, genclass, argnames):
        if self._is_static:
            return 'self->box = new Box(%s::%s(%s));' % (genclass.classname, self._funcname, argnames)
        return 'self->box = new Box(sp<%s>::make(%s));' % (self._funcname, argnames)

    def gen_py_return(self):
        return 'int'

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* args, PyObject* kws'

    @property
    def err_return_value(self):
        return 'return -1'

    def need_unpack_statement(self):
        return False

    def gen_py_type_constructor_codes(self, lines, genclass):
        lines.append('pyTypeObject->tp_init = reinterpret_cast<initproc>(%s::__init___r);' % genclass.py_class_name)


class GenPropertyMethod(GenMethod):
    def __init__(self, name, args, return_type, is_static):
        GenMethod.__init__(self, name, args, return_type, is_static)
        if is_static:
            self._self_argument = self._arguments and self._arguments[0]
            self._arguments = self._arguments[1:]
        self._is_setter = name.startswith('set')
        self._property_name = name[3].lower() + name[4:] if self._is_setter else name

    def gen_py_return(self):
        return 'int' if self._is_setter else "PyObject*"

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    def _gen_convert_args_code(self, lines, argdeclare, optional_check=False):
        if argdeclare:
            arg0 = self._arguments[0]
            meta = GenArgumentMeta('PyObject*', arg0.accept_type, 'O')
            ga = GenArgument(0, arg0.accept_type, arg0.default_value, meta, str(arg0))
            lines.append(ga.gen_declare('obj0', 'arg0', optional_check=optional_check))

    def gen_py_arguments(self):
        if self._is_setter:
            return 'Instance* self, PyObject* arg0, void* /*closure*/'
        return 'Instance* self, PyObject* /*args*/'

    def gen_py_argc(self):
        if self._is_setter:
            return 1
        return 0

    def gen_py_getset_def(self, properties, genclass):
        trycatch_suffix = '_r' if 't' in get_params() else ''
        property_def = self._ensure_property_def(properties)
        func = f'{genclass.py_class_name}::{self._name}{trycatch_suffix}'
        if self._is_setter:
            property_def[2] = f'(setter) {func}'
        else:
            property_def[1] = f'(getter) {func}'

    def _gen_calling_statement(self, genclass, argnames):
        self_statement = self.gen_self_statement(genclass)
        if self._is_static:
            lines = ['%s::%s(%s%s);' % (genclass.classname, self._name, self_statement, argnames and ', ' + argnames)]
        else:
            lines = ['%s->%s(%s);' % (self_statement, self._name, argnames)]
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

    def overload(self, m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(GenPropertyMethod, is_static=m1.is_static)(m1, m2)


class GenLoaderMethod(GenMethod):
    def __init__(self, name, args):
        GenMethod.__init__(self, name, ['TypeId typeId'] + args, 'PyObject*')

    def gen_py_method_def(self, genclass):
        return self.gen_py_method_def_tp(genclass)

    def _gen_calling_statement(self, genclass, argnames):
        return 'PythonInterpreter::instance().ensurePyArkType(reinterpret_cast<PyObject*>(self))->load(*self, "%s", %s);' % (self._name, argnames)

    def need_unpack_statement(self):
        return False

    def gen_loader_statement(self, classname, beanname, methodname):
        return acg.format('loader[Type<${beanname}>::id()] = PyArk${classname}_${methodname}Function<${beanname}>;', classname=classname, beanname=beanname, methodname=methodname)


class GenMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_method_def(self, genclass):
        return self.gen_py_method_def_tp(genclass)


class GenStaticMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)
        self._flags = self._flags + '|METH_STATIC'

    def gen_py_method_def(self, genclass):
        return self.gen_py_method_def_tp(genclass)

    def need_unpack_statement(self):
        return False

    def _gen_calling_statement(self, genclass, argnames):
        return '%s::%s(%s);' % (genclass.classname, self._name, argnames)


class GenStaticMemberMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)
        self._self_argument = self._arguments and self._arguments[0]
        self._arguments = self._arguments[1:]

    def gen_py_method_def(self, genclass):
        return self.gen_py_method_def_tp(genclass)

    def _gen_calling_statement(self, genclass, argnames):
        return '%s::%s(%s%s);' % (genclass.classname, self._name, self.gen_self_statement(genclass), argnames if not argnames else ', ' + argnames)


class GenRichCompareMethod(GenMethod):
    def __init__(self, name, args, return_type, operator):
        GenMethod.__init__(self, name, args, return_type)
        self._self_argument = self._arguments and self._arguments[0]
        self._arguments = self._arguments[1:]
        self._operator = operator

    def gen_py_return(self):
        return 'PyObject*'

    def _gen_calling_statement(self, genclass, argnames):
        return '%s::%s(%s%s);' % (genclass.classname, self._name, self.gen_self_statement(genclass), argnames if not argnames else ', ' + argnames)

    @staticmethod
    def _gen_convert_args_code(lines, argdeclare, optional_check=False):
        pass

    def _gen_parse_tuple_code(self, lines, declares, args):
        if args:
            meta = GenArgumentMeta('PyObject*', args[0].accept_type, 'O')
            ga = GenArgument(0, args[0].accept_type, args[0].default_value, meta, str(args[0]))
            lines.append(ga.gen_declare('obj0', 'args'))

    @property
    def operator(self):
        return self._operator


class GenClass(object):
    def __init__(self, filename, class_name, is_container=False):
        self._py_src_name = 'py_ark_' + acg.camel_case_to_snake_case(class_name) + '_type'
        self._py_class_name = 'PyArk%sType' % class_name
        self._filename = filename
        self._name = class_name
        self._classname = class_name
        self._binding_classname = class_name
        self._base_classname = None
        self._is_container = is_container
        self._methods = {}
        self._enum_constants = {}

    @property
    def classname(self):
        return self._classname

    @property
    def name(self):
        return self._name

    @name.setter
    def name(self, v):
        self._name = v

    @property
    def binding_classname(self):
        return self._binding_classname

    @binding_classname.setter
    def binding_classname(self, v):
        self._py_src_name = 'py_ark_' + acg.camel_case_to_snake_case(v) + '_type'
        self._py_class_name = 'PyArk%sType' % v
        self._binding_classname = v
        self._name = v

    @property
    def base_classname(self):
        return self._base_classname

    @base_classname.setter
    def base_classname(self, base_classname):
        self._base_classname = base_classname

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
    def enum_constants(self):
        return self._enum_constants

    @property
    def filename(self):
        return self._filename

    @property
    def is_container(self):
        return self._is_container

    def add_method(self, method):
        try:
            m = self._methods[method.name]
            can_overload = hasattr(m, 'add_overloaded_method') or type(m) is type(method)
            assert can_overload, f'Trying to overload two different methods "{type(m).__name__}" and "{type(method).__name__}"'
            method = m.overload(m, method)
        except KeyError:
            pass
        self._methods[method.name] = method

    def add_enum_constant(self, name, value):
        self._enum_constants[name] = value

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

    def setprop_methods(self):
        return self.find_methods_by_type(GenSetPropMethod)

    def subscribe_methods(self):
        return self.find_methods_by_type(GenMappingMethod)

    def sequence_methods(self):
        return self.find_methods_by_type(GenSequenceMethod)

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

    def gen_rich_compare_defs(self):
        rich_compare_defs = []
        rich_compare_methods = self.rich_compare_methods()
        if rich_compare_methods:
            cases = []
            for i in rich_compare_methods:
                try:
                    cases.append('    case %s:' % RICH_COMPARE_OPS[i.operator])
                    cases.append('        return PyCast::toPyObject(%s::%s(unpacked, obj0));' % (self._classname, i.name))
                except KeyError:
                    pass
            rich_compare_defs.extend([
                '',
                'static PyObject* %s_tp_richcompare (PyArkType::Instance* self, PyObject* args, int op) {' % self._py_class_name,
                '    const sp<%s> unpacked = self->as<%s>();' % (self._binding_classname, self._binding_classname),
                '    const sp<%s> obj0 = PyCast::ensureSharedPtr<%s>(args);' % (self._binding_classname, self._binding_classname),
                '    switch(op) {'] + cases + [
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
    binding_classes = {}
    bindables = set(ARK_CORE_BUILDABLES)

    namespaces = params['p'].replace('::', ':').split(':')
    modulename = params['m']
    output_file = params['o'] if 'o' in params else None
    output_dir = params['d'] if 'd' in params else None
    output_cmakelist = params['c'] if 'c' in params else None
    bindable_paths = params['b'] if 'b' in params else None

    def automethod(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
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
        genclass = get_result_class(binding_classes, filename, main_class)
        operator = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        if operator in RICH_COMPARE_OPS:
            assert is_static
            genclass.add_method(GenRichCompareMethod(name, args, return_type, operator))
        else:
            genclass.add_method(GenOperatorMethod(name, args, return_type, operator, is_static))

    def autoasmapping(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        operator = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        genclass.add_method(GenMappingMethod(name, args, return_type, operator, is_static))

    def autoassequence(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        operator = x[0]
        name, args, return_type, is_static = GenMethod.split(x[1:])
        genclass.add_method(GenSequenceMethod(name, args, return_type, operator, is_static))

    def autoclass(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        args = [i.strip() for i in x[1].replace('"', '').split(',')]
        assert x[0] in ('class', 'name') and len(args) in (1, 2)
        if x[0] == 'class':
            genclass.binding_classname = args[0]
            if len(args) > 1:
                genclass.name = args[1]
        else:
            genclass.name = args[0]

    def autoextends(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        genclass.base_classname = x

    class AutoMethodCall:
        def __init__(self, auto_method_type, argc=4):
            self._auto_method_type = auto_method_type
            self._argc = argc

        def __call__(self, filename, content, main_class, x):
            genclass = get_result_class(binding_classes, filename, main_class)
            splitted = GenMethod.split(x)
            genclass.add_method(self._auto_method_type(*splitted[:self._argc]))

    def autoloader(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenLoaderMethod(name, args))

    def autoproperty(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenPropertyMethod(name, args, return_type, is_static))

    def autogetprop(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        name, args, return_type, is_static = GenMethod.split(x)
        genclass.add_method(GenGetPropMethod(name, args, return_type))

    def autoannotation(filename, content, m, x):
        names = [i for i in x[1].split(':', 2)[0].split() if i not in ('ARK_API', 'final')]
        if len(names) >= 1:
            binding_classes[names[-1]] = GenClass(filename, names[-1], x[0].strip() in ('container', 'holder'))

    def autotypedef(filename, content, m, x):
        binding_classes[x] = GenClass(filename, x, False)

    def autobindable(filename, content, main_class, x):
        bindables.add(x)

    def autoenumeration(filename, content, main_class, x):
        genclass = get_result_class(binding_classes, filename, main_class)
        for i in x[1].split(',\n'):
            varname = i.split('=')[0].strip()
            if varname:
                genclass.add_enum_constant(varname, '%s::%s' % (main_class, varname))

    acg.match_header_patterns(paths, True,
                              HeaderPattern(AUTOBIND_ANNOTATION_PATTERN, autoannotation),
                              HeaderPattern(AUTOBIND_TYPEDEF_PATTERN, autotypedef),
                              HeaderPattern(AUTOBIND_METHOD_PATTERN, automethod),
                              HeaderPattern(AUTOBIND_OPERATOR_PATTERN, autooperator),
                              HeaderPattern(AUTOBIND_AS_MAPPING_PATTERN, autoasmapping),
                              HeaderPattern(AUTOBIND_AS_SEQUENCE_PATTERN, autoassequence),
                              HeaderPattern(AUTOBIND_CLASS_PATTERN, autoclass),
                              HeaderPattern(AUTOBIND_EXTENDS_PATTERN, autoextends),
                              HeaderPattern(AUTOBIND_LOADER_PATTERN, AutoMethodCall(GenLoaderMethod, 2)),
                              HeaderPattern(AUTOBIND_PROPERTY_PATTERN, AutoMethodCall(GenPropertyMethod)),
                              HeaderPattern(AUTOBIND_GETPROP_PATTERN, AutoMethodCall(GenGetPropMethod, 3)),
                              HeaderPattern(AUTOBIND_SETPROP_PATTERN, AutoMethodCall(GenSetPropMethod, 3)),
                              HeaderPattern(AUTOBIND_ENUMERATION_PATTERN, autoenumeration),
                              HeaderPattern(BUILDABLE_PATTERN, autobindable))

    acg.match_header_patterns(paths, False,
                              HeaderPattern(AUTOBIND_TYPEDEF_PATTERN, autotypedef))

    if bindable_paths:
        acg.match_header_patterns(bindable_paths.split(path.pathsep), True, HeaderPattern(BUILDABLE_PATTERN, autobindable))

    if output_cmakelist:
        return acg.write_to_file(_just_print_flag and output_cmakelist, gen_cmakelist_source(params, paths, output_dir, output_file or 'stdout', binding_classes))

    gen_bindingcpp = output_file or _just_print_flag is None
    head_src = gen_header_source(output_file or 'stdout', output_dir, output_file, binding_classes, namespaces)
    cpp_src = gen_body_source(output_file or 'stdout', output_dir, output_file, namespaces, modulename, binding_classes, bindables, gen_bindingcpp)
    if gen_bindingcpp:
        acg.write_to_file(_just_print_flag and output_file + '.h', head_src)
        acg.write_to_file(_just_print_flag and output_file + '.cpp', cpp_src)


if __name__ == '__main__':
    # opts, paths = getopt.getopt(sys.argv[1:], 'b:c:p:n:o:l:m:d:j')
    # params = dict((i.lstrip('-'), j) for i, j in opts)
    # if any(i not in params for i in ['p', 'm']) or len(paths) == 0:
    #     print('Usage: %s -p namespace -m modulename [-c cmakefile] [-o output_file] [-l library_path] dir_paths...' % sys.argv[0])
    #     sys.exit(0)
    #
    # library_path = params['l'] if 'l' in params else None
    # if library_path and library_path not in sys.path:
    #     sys.path.append(library_path)
    params, paths = get_param_and_paths()

    acg = import_acg()

    from acg import HeaderPattern

    if 'j' in params:
        _just_print_flag = None

    main(params, paths)
