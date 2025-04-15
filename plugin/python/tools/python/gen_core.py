import getopt
import importlib
import re
import sys
from dataclasses import dataclass
from typing import Optional

acg = None

_params = None
_paths = None

INDENT = '    '

TYPE_DEFINED_OBJ = ('V2', 'V3', 'V4', 'M3', 'M4', 'NamedHash', 'Rect', 'RectI', 'RectF', 'Slice', 'PyInstance')
TYPE_DEFINED_SP = ('document', 'element', 'attribute', 'bitmap')
TYPE_COLLECTION_TEMPLATE_PREFIX = ['std::vector<', 'std::map<', 'Map<', 'HashMap<', 'Vector<', 'Set<']


def is_collection_template_type(typename: str):
    return any(typename.startswith(i) for i in TYPE_COLLECTION_TEMPLATE_PREFIX)


def get_param_and_paths() -> tuple[dict[str, str], list[str]]:
    global _paths, _params

    if _params is None and _paths is None:
        opts, _paths = getopt.getopt(sys.argv[1:], 'b:c:p:n:o:l:m:d:jt')
        _params = dict((i.lstrip('-'), j) for i, j in opts)
        if any(i not in _params for i in ['p', 'm']) or len(_paths) == 0:
            print('Usage: %s -p namespace -m modulename [-c cmakefile] [-o output_file] [-l library_path] dir_paths...' % sys.argv[0])
            sys.exit(0)

        library_path = _params['l'] if 'l' in _params else None
        if library_path and library_path not in sys.path:
            sys.path.append(library_path)

        global acg
        acg = import_acg()

    return _params, _paths


def get_params() -> dict[str, str]:
    return _params


def import_acg():
    return importlib.import_module('acg')


get_param_and_paths()


def parse_method_arguments(arguments):
    args = []
    for i, j in enumerate(arguments):
        argstr = acg.remove_crv(j)
        argstr, _, default_value = argstr.strip().partition('=')
        if default_value:
            if not argstr.startswith('Scope'):
                default_value = default_value.strip()
                if default_value.endswith('('):
                    default_value += ')'
            argstr = argstr.strip()
        argsplit = argstr.split()
        argtype = ' '.join(argsplit[:-1])
        argname = argsplit[-1]
        for k, l in ARK_PY_ARGUMENTS:
            m = re.match(k, argtype)
            if m:
                cast_signature = acg.format(l.cast_signature, *m.groups())
                typename = acg.format(l.typename, *m.groups())
                argument_meta = GenArgumentMeta(typename, cast_signature, l.parse_signature, l.is_base_type, l.has_defvalue, l.movable)
                args.append(GenArgument(i, cast_signature, None if default_value == '' else default_value, argument_meta, argtype, argname))
                break
        else:
            print(f'Undefined method argument: "{arguments[i]}"')
            sys.exit(-1)
    return args


def remove_crv(typename):
    t1 = acg.strip_key_words(typename, ['const', 'volatile']).strip()
    return (t1[:-1] if t1.endswith('&') else t1).strip()


def gen_cast_call(targettype: str, name: str) -> str:
    if targettype == 'bool':
        return f'{name} != 0'
    return f'static_cast<{targettype}>({name})'


def gen_method_call_arg(name: str, gen_arg, argtype: str):
    targettype = acg.remove_crv(gen_arg.str())
    argtype = acg.remove_crv(argtype)
    if argtype.startswith('sp<') and acg.get_shared_ptr_type(argtype) == targettype:
        return f'*{name}'
    if targettype != argtype:
        return gen_cast_call(targettype, name)
    if gen_arg.meta.movable or any(targettype.startswith(i) for i in (TYPE_COLLECTION_TEMPLATE_PREFIX + ['sp<', 'Box', 'Traits'])):
        return f'std::move({name})'
    return name
    # if ctype in ARK_PY_ARGUMENT_CHECKERS:
    #     return ARK_PY_ARGUMENT_CHECKERS[ctype].cast(argname)
    # return argname


class GenConverter:
    def __init__(self, check_func: str):
        self._check_func = check_func

    def check(self, var):
        return self._check_func.format(var)


ARK_PY_OVERLOADED_ARGUMENT_TYPE_CHECKERS = {
    'bool': GenConverter('!PythonExtension::instance().isInstance<Boolean>({0})'),
    'int32_t': GenConverter('PyBridge::isPyLongExact({0})'),
    'uint32_t': GenConverter('PyBridge::isPyLongExact({0})'),
    'float': GenConverter('(PyBridge::isPyLongExact({0}) || PyBridge::isPyFloatExact({0}))'),
    'std::vector<float>': GenConverter('(PyBridge::isPySequence({0})'),
    'std::vector<int32_t>': GenConverter('(PyBridge::isPySequence({0})'),
    'V2': GenConverter('(PyBridge::isPySequence({0}) && PyBridge::PyObject_Size({0}) == 2)'),
    'V3': GenConverter('(PyBridge::isPySequence({0}) && PyBridge::PyObject_Size({0}) == 3)'),
    'V4': GenConverter('(PyBridge::isPySequence({0}) && PyBridge::PyObject_Size({0}) == 4)'),
    'std::wstring': GenConverter('PyBridge::isPyUnicodeExact({0})')
}


@dataclass
class GenArgumentMeta:
    typename: str
    cast_signature: str
    parse_signature: str
    is_base_type: bool = False
    has_defvalue: bool = False
    movable: bool = False


ARK_PY_ARGUMENTS = (
    (r'String\s*&?', GenArgumentMeta('const char*', 'const char*', 's')),
    (r'Scope\s*&', GenArgumentMeta('PyObject*', 'Scope', '**')),
    (r'Traits\s*&?', GenArgumentMeta('PyObject*', 'Traits', '*')),
    (r'std::wstring\s*&?', GenArgumentMeta('PyObject*', 'std::wstring', 'O')),
    (r'Box\s*&?', GenArgumentMeta('PyObject*', 'Box', 'O')),
    (r'sp<([^>]+|\w+<\w+>)>(?:\s*&|$)', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'(Optional<[^\s]+>)(?:\s*&)?', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (r'(document|element|attribute)\s*&', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (r'(V2|V3|V4)', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (rf'((?:{"|".join(TYPE_COLLECTION_TEMPLATE_PREFIX)}|[^:]+::).+)', GenArgumentMeta('PyObject*', '${0}', 'O')),
    (r'(^(?:Buffer|LayoutLength)$)', GenArgumentMeta('PyObject*', '${0}', 'O', movable=True)),
    (r'([^>]+|\w+<\w+>)\s*&', GenArgumentMeta('PyObject*', 'sp<${0}>', 'O')),
    (r'(uint32_t|RefId|unsigned int|uint8_t)', GenArgumentMeta('uint32_t', 'uint32_t', 'I')),
    (r'size_t', GenArgumentMeta('size_t', 'size_t', 'n')),
    (r'ptrdiff_t', GenArgumentMeta('ptrdiff_t', 'ptrdiff_t', 'i')),
    (r'int64_t', GenArgumentMeta('int64_t', 'int64_t', 'L')),
    (r'uint64_t', GenArgumentMeta('uint64_t', 'uint64_t', 'K')),
    (r'(int32_t|int16_t|int8_t)', GenArgumentMeta('int32_t', 'int32_t', 'i')),
    (r'float', GenArgumentMeta('float', 'float', 'f')),
    (r'bool', GenArgumentMeta('int32_t', 'bool', 'p', True)),
    (r'HashId|TypeId', GenArgumentMeta('PyObject*', 'uint32_t', 'O'))
)


class GenArgument:
    def __init__(self, index: int, accept_type: str, default_value: str, meta: GenArgumentMeta, argtype: str, argname: Optional[str] = None):
        self._index = index
        self._accept_type = accept_type
        self._default_value = default_value
        self._meta = meta
        self._str = argtype
        self._argname = argname

    @property
    def index(self):
        return self._index

    @property
    def argname(self):
        return self._argname

    @property
    def accept_type(self):
        return self._accept_type

    @property
    def default_value(self) -> str:
        return self._default_value

    @property
    def meta(self):
        return self._meta

    @property
    def typename(self):
        return self._meta.typename

    @property
    def parse_signature(self) -> str:
        s = self._meta.parse_signature
        if s.startswith('*'):
            return ''
        return '|' + s if self.has_defvalue else s

    @property
    def has_defvalue(self):
        return self._default_value or self._meta.has_defvalue

    def type_compare(self, *typenames) -> bool:
        return any(acg.type_compare(i, self._str) for i in typenames)

    def gen_type_check(self, varname):
        if self._accept_type in ARK_PY_OVERLOADED_ARGUMENT_TYPE_CHECKERS:
            return "%s && %s" % (varname, ARK_PY_OVERLOADED_ARGUMENT_TYPE_CHECKERS[self._accept_type].check(varname))
        return None

    def gen_declare(self, objname, argname, extract_cast=False, optional_check=False):
        typename = self._meta.cast_signature
        if self._meta.is_base_type:
            return '%s %s = %s;' % (typename, objname, gen_cast_call(typename, argname))
        if self.typename == self._meta.cast_signature:
            return '%s %s = %s;' % (typename, objname, argname)
        m = acg.get_shared_ptr_type(self._accept_type)
        if m == 'Scope':
            return f'const Scope {objname} = PyCast::toScope(kws);'
        elif m == 'Traits':
            return f'Traits {objname} = PyCast::toTraits(args, {self._index});'

        is_optional_type = 'Optional<' in m
        optional_cast_prefix = 'to' if optional_check or is_optional_type else 'ensure'
        to_cpp_object = '%sCppObject' % optional_cast_prefix
        if m == 'char*':
            return self._gen_var_declare('String', objname, to_cpp_object, 'String', argname, False, optional_check)
        if m in TYPE_DEFINED_OBJ:
            return self._gen_var_declare(m, objname, to_cpp_object, m, argname, False, optional_check)
        if m in TYPE_DEFINED_SP:
            return self._gen_var_declare(m, objname, to_cpp_object, m, argname, False, optional_check)
        typename = acg.remove_crv(typename)
        if m != self._accept_type and not is_collection_template_type(typename):
            return self._gen_var_declare('sp<%s>' % m, objname, '%sSharedPtr' % optional_cast_prefix, m, argname, extract_cast, optional_check)
        functype = acg.get_template_type(typename, 'Optional') if is_optional_type else typename
        return self._gen_var_declare(typename, objname, to_cpp_object, functype, argname, False, optional_check)

    def _gen_var_declare(self, typename, varname, funcname, functype, argname, extract_cast=False, optional_check=False):
        if optional_check:
            typename = f'Optional<{typename}>'
        if self._default_value and (self._accept_type.startswith('sp<') or self._meta.parse_signature == 'O'):
            return '%s %s = %s ? PyCast::%s<%s>(%s) : %s;' % (
                typename, varname, argname, funcname, functype, argname, self._gen_default_value_declare(typename, optional_check))
        return f'{typename} {varname} = PyCast::{funcname}<{functype}>({argname});'

    def _gen_default_value_declare(self, typename: str, optional_check: bool):
        if self._default_value.startswith('{') and self._default_value.endswith('}'):
            return f'{typename}{self._default_value}'
        return f'{typename}({self._default_value})' if optional_check else self._default_value

    def str(self):
        return self._str

    def __str__(self):
        return self.str()


def create_overloaded_method_type(base_type, **kwargs):

    class GenOverloadedMethod(base_type):
        def __init__(self, m1, m2):
            base_type.__init__(self, m1.name, [], m1.return_type, **kwargs)
            self._arguments = self._replace_arguments(m1.arguments, m2.arguments)
            self._overloaded_methods = []
            self.add_overloaded_method(m1)
            self.add_overloaded_method(m2)

        def gen_definition_body(self, genclass, lines, arguments, gen_type_check_args, exact_cast, check_args, optional_check=False):
            not_overloaded_args = [i for i in self._arguments]

            for i in self._overloaded_methods:
                for j, k in enumerate(not_overloaded_args):
                    if k and i.arguments[j].accept_type != k.accept_type:
                        not_overloaded_args[j] = None

            m0 = self._overloaded_methods[0]
            not_overloaded_declar = [j.gen_declare('obj%d' % i, 'arg%d' % i, not m0.check_argument_type) for i, j in enumerate(not_overloaded_args) if j]
            self._gen_convert_args_code(lines, not_overloaded_declar)
            for i in self._overloaded_methods:
                type_checks = [k.gen_type_check('arg%d' % j) for j, k, l in zip(range(len(i.arguments)), i.arguments, not_overloaded_args) if not l]
                lines.extend(['if(%s)' % ' && '.join(([j for j in type_checks if j]) or ['true']), '{'])
                body_lines = []
                overloaded_args = [None if j else k for j, k in zip(not_overloaded_args, i.arguments)]
                i.gen_definition_body(genclass, body_lines, overloaded_args, overloaded_args, True, False, True)
                lines.extend(INDENT + j for j in body_lines)
                lines.append('}')
            return_type = m0.err_return_value
            if m0.check_argument_type:
                lines.append('PyBridge::setTypeErrString(Strings::sprintf("Calling overloaded method(%%s::%%s) failed, no arguments matched", "%s", "%s").c_str());' % (genclass.name, self._name))
            lines.append(return_type + ';')

        def add_overloaded_method(self, method):
            if self._overloaded_methods:
                m1 = self._overloaded_methods[-1]
                if len(m1.arguments) != len(method.arguments):
                    print('Overloaded methods(%s, %s) should have equal number of arguments' % (m1, method))
                    sys.exit(-1)
            method._arguments = self._replace_arguments(method.arguments, self._arguments)
            self._overloaded_methods.append(method)

        def _replace_arguments(self, args1: list[GenArgument], args2: list[GenArgument]):
            assert len(args1) == len(args2)
            assert all(i.argname == j.argname for i, j in zip(args1, args2)), f"Overloaded methods({self.name}) should have the same argument names"
            return [GenArgument(i.index, i.accept_type, i.default_value, GenArgumentMeta('PyObject*', i.meta.cast_signature, 'O', False, i.has_defvalue or j.has_defvalue), i.str(), i.argname) for i, j in zip(args1, args2)]

    return GenOverloadedMethod
