import sys

from gen_core import parse_method_arguments, acg, gen_cast_call, INDENT, gen_method_call_arg, GenArgumentMeta, GenArgument, \
    create_overloaded_method_type, get_params

TP_AS_NUMBER_TEMPLATE = [
    'static PyNumberMethods ${py_class_name}_tp_as_number = {',
    '    (binaryfunc) ${nb_add},        /* binaryfunc nb_add;                  */ /* __add__ */',
    '    (binaryfunc) ${nb_subtract}, /* binaryfunc nb_subtract; */ /* __sub__ */',
    '    (binaryfunc) ${nb_multiply}, /* binaryfunc nb_multiply; */ /* __mul__ */',
    '    (binaryfunc) ${nb_remainder}, /* binaryfunc nb_remainder; */ /* __mod__ */',
    '    nullptr,               /* binaryfunc nb_divmod; */ /* __divmod__ */',
    '    (ternaryfunc) ${nb_power},   /* ternaryfunc nb_power; */ /* __pow__ */',
    '    (unaryfunc) ${nb_negative},        /* unaryfunc nb_negative; */ /* __neg__ */',
    '    nullptr,               /* unaryfunc nb_positive; */ /* __pos__ */',
    '    (unaryfunc) ${nb_absolute},               /* unaryfunc nb_absolute; */ /* __abs__ */',
    '    (inquiry) ${nb_bool}, /* inquiry nb_bool; */ /* __bool__ */',
    '    nullptr,               /* unaryfunc nb_invert; */ /* __invert__ */',
    '    nullptr,               /* binaryfunc nb_lshift; */ /* __lshift__ */',
    '    nullptr,               /* binaryfunc nb_rshift; */ /* __rshift__ */',
    '    (binaryfunc) ${nb_and}, /* binaryfunc nb_and; */ /* __and__ */',
    '    nullptr,               /* binaryfunc nb_xor; */ /* __xor__ */',
    '    (binaryfunc) ${nb_or}, /* binaryfunc nb_or; */ /* __or__ */',
    '    (unaryfunc) ${nb_int}, /* unaryfunc nb_int; */ /* __int__ */',
    '    nullptr,               /* void *nb_reserved; */',
    '    (unaryfunc) ${nb_float},       /* unaryfunc nb_float; */ /* __float__ */',
    '    (binaryfunc) ${nb_inplace_add},       /* binaryfunc nb_inplace_add */',
    '    (binaryfunc) ${nb_inplace_subtract},       /* binaryfunc nb_inplace_subtract */',
    '    (binaryfunc) ${nb_inplace_multiply},       /* binaryfunc nb_inplace_multiply */',
    '    nullptr, /* binaryfunc nb_inplace_remainder;  */',
    '    nullptr, /* ternaryfunc nb_inplace_power;     */',
    '    nullptr, /* binaryfunc nb_inplace_lshift;     */',
    '    nullptr, /* binaryfunc nb_inplace_rshift;     */',
    '    nullptr, /* binaryfunc nb_inplace_and;        */',
    '    nullptr, /* binaryfunc nb_inplace_xor;        */',
    '    nullptr, /* binaryfunc nb_inplace_or;         */',
    '    (binaryfunc) ${nb_floor_divide}, /* binaryfunc nb_floor_divide;       */',
    '    (binaryfunc) ${nb_true_divide}, /* binaryfunc nb_true_divide;         */ /* __div__ */',
    '    nullptr, /* binaryfunc nb_inplace_floor_divide;*/',
    '    (binaryfunc) ${nb_inplace_true_divide}, /* binaryfunc nb_inplace_true_divide  */ /* __idiv__ */',
    '    (unaryfunc) ${nb_index}, /* unaryfunc nb_index;  */ /* __index__ */',
    '    (binaryfunc) ${nb_matrix_multiply}, /* binaryfunc nb_matrix_multiply */ /* __matmul__ */',
    '    (binaryfunc) ${nb_inplace_matrix_multiply} /* binaryfunc nb_inplace_matrix_multiply */ /* __imatmul__ */',
    '};'
]


TP_AS_NUMBER_TEMPLATE_OPERATOR = {
    '+': 'nb_add',
    '-': 'nb_subtract',
    '*': 'nb_multiply',
    '%': 'nb_remainder',
    'bool': 'nb_bool',
    'neg': 'nb_negative',
    'abs': 'nb_absolute',
    'pow': 'nb_power',
    '&': 'nb_and',
    '|': 'nb_or',
    'int': 'nb_int',
    'index': 'nb_index',
    'float': 'nb_float',
    '+=': 'nb_inplace_add',
    '-=': 'nb_inplace_subtract',
    '*=': 'nb_inplace_multiply',
    '//': 'nb_floor_divide',
    '/': 'nb_true_divide',
    '/=': 'nb_inplace_true_divide',
    '@': 'nb_matrix_multiply',
    '@=': 'nb_inplace_matrix_multiply'
}


PY_RETURN_NONE = 'return PyBridge::incRefNone()'


class GenMethod(object):
    def __init__(self, name, args, return_type, is_static: bool = False):
        self._name = name
        self._return_type = return_type
        self._is_static = is_static
        self._arguments = parse_method_arguments(args)
        self._has_args_argument = args and self._arguments[-1].type_compare('Traits')
        self._has_kwargs_argument = args and self._arguments[-1].type_compare('Scope')
        self._has_scope_or_traits_argument = self._has_args_argument or self._has_kwargs_argument
        self._has_keyword_arguments = self._has_scope_or_traits_argument or self._has_keyword_names()
        self._flags = ('METH_VARARGS|METH_KEYWORDS' if self._has_keyword_arguments else 'METH_VARARGS')
        self._self_argument = None

    @property
    def is_static(self):
        return self._is_static

    @property
    def self_argument(self):
        return self._self_argument

    def gen_py_method_def(self, genclass):
        return None

    def gen_py_method_def_tp(self, genclass):
        trycatch_suffix = '_r' if 't' in get_params() else ''
        return f'{{"{acg.camel_case_to_snake_case(self._name)}", (PyCFunction) {genclass.py_class_name}::{self._name}{trycatch_suffix}, {self._flags}, nullptr}}'

    def gen_py_return(self):
        return 'PyObject*'

    def gen_py_arguments(self):
        return f'Instance* self, PyObject* args{", PyObject* kws" if self._has_keyword_arguments else ""}'

    def gen_py_argc(self):
        return 'PyBridge::PyObject_Size(args)'

    def gen_local_var_declarations(self):
        declares = {}
        for i, j in enumerate(self._arguments):
            if j.meta.parse_signature == '*':
                continue

            typename = j.typename
            realtypename = acg.remove_crv(typename)
            if typename in declares:
                declares[typename] = '%s, %sarg%d' % (declares[typename], '*' if typename.endswith('*') else '', i)
            else:
                declares[typename] = '%s arg%d' % (typename, i)
            if j.meta.parse_signature == '**':
                declares[typename] += ' = kws'
            else:
                dvalue = None
                if realtypename == 'PyObject*':
                    if j.accept_type == 'bool':
                        dvalue = 'Py_True' if j.default_value == 'true' else 'Py_False'
                    else:
                        dvalue = 'nullptr'
                elif j.default_value is not None:
                    dmap = {'true': '1', 'false': '0'}
                    if j.typename in ('uint32_t', 'int32_t', 'uint64_t', 'int64_t', 'float', 'size_t'):
                        dvalue = j.default_value if j.default_value not in dmap else dmap[j.default_value]
                    elif realtypename == 'bool' and j.accept_type == 'bool':
                        dvalue = 'true' if j.default_value == 'true' else 'false'
                    elif realtypename == 'char*' and j.default_value:
                        dvalue = j.default_value
                    else:
                        dvalue = 'nullptr'
                if dvalue is not None:
                    declares[typename] += ' = %s' % dvalue
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
        return PY_RETURN_NONE

    @property
    def check_argument_type(self):
        return True

    def _gen_calling_statement(self, genclass, argnames):
        return 'unpacked->%s(%s);' % (self._name, argnames)

    def _gen_calling_body(self, genclass, calling_lines):
        return calling_lines

    def gen_return_statement(self, return_type: str, py_return: str):
        if acg.type_compare(return_type, py_return):
            return ['return ret;']

        if py_return != 'PyObject*':
            return [f'return {0 if return_type == "void" else gen_cast_call(py_return, "ret")};']

        if return_type == 'void':
            return [f'{PY_RETURN_NONE};']
        m = acg.get_shared_ptr_type(return_type)
        if m in ('std::wstring',):
            fromcall = 'template toPyObject<%s>' % m
        else:
            fromcall = 'toPyObject'
        return ['return PyCast::%s(ret);' % fromcall]

    def _gen_parse_tuple_code(self, lines: list[str], declares: list[str], args: list[GenArgument]):
        lines.append(f'\n{INDENT}'.join(declares))
        parse_format = ''.join(i.parse_signature for i in args)
        if parse_format.count('|') > 1:
            ts = parse_format.split('|')
            if any(len(i) > 1 for i in ts[1:]):
                print('Illegal default arguments: %s', parse_format)
                sys.exit(-1)
            parse_format = ts[0] + '|' + ''.join(ts[1:])

        parse_arg_refnames = ', '.join(f'&arg{i}' for i, j in enumerate(args) if j.parse_signature)
        if self._has_keyword_names() and not self._has_scope_or_traits_argument:
            lines.append(self._make_argname_declares())
            parsestatement = f'''if(!PyBridge::PyArg_ParseTupleAndKeywords(args, kws, "{parse_format}", const_cast<char**>(argnames), {parse_arg_refnames}))
        {self.err_return_value};
    '''
        else:
            args_name, args_declare = self._gen_parse_tuple_args()
            parsestatement = f'''{args_declare}if(!PyBridge::PyArg_ParseTuple({args_name}, "{parse_format}", {parse_arg_refnames}))
        {self.err_return_value};
    '''
        if parse_format:
            lines.append(parsestatement)

    def _gen_parse_tuple_args(self) -> (str, str):
        if self._has_args_argument:
            args_idx = len(self._arguments) - 1
            args_name = f'args0_{args_idx}'
            return f'{args_name}.pyObject()', f'const PyInstance {args_name} = PyInstance::steal(PyBridge::PyTuple_GetSlice(args, 0, {args_idx}));\n    '
        return 'args', ''

    def _make_argname_declares(self):
        return '''const char* argnames[] = {
        %s%s
        nullptr
    };''' % (f',\n{INDENT * 2}'.join(f'"{acg.camel_case_to_snake_case(i.argname)}"' for i in self._arguments), ',' if self._arguments else '')

    def _has_keyword_names(self) -> bool:
        return all(i.argname is not None for i in self._arguments)

    def need_unpack_statement(self):
        return not (self._self_argument and self._self_argument.type_compare('Box'))

    def is_constructor(self):
        return False

    def gen_self_statement(self, genclass):
        if self._self_argument:
            if self._self_argument.type_compare('Box'):
                return '*self->box'
            if not self._self_argument.type_compare(f'sp<{genclass.binding_classname}>', f'{genclass.binding_classname}&'):
                return f'unpacked.cast<{acg.get_shared_ptr_type(self._self_argument.accept_type)}>()'
        return 'unpacked'

    def gen_declaration(self):
        return [
            f'static {self.gen_py_return()} {self._name}({self.gen_py_arguments()});',
            f'constexpr static auto {self._name}_r = ark::plugin::python::PyCast::RuntimeFuncWrapper<decltype(&{self._name}), {self._name}>;'
        ]

    @staticmethod
    def _gen_convert_args_code(lines, argdeclare, optional_check=False):
        if argdeclare:
            lines.extend(argdeclare)

    def gen_definition(self, genclass):
        lines = []

        if len(self._arguments) > 0:
            self._gen_parse_tuple_code(lines, self.gen_local_var_declarations(), self._arguments)

        if self.need_unpack_statement():
            class_name = genclass.binding_classname
            lines.append(f'sp<{class_name}> unpacked = self->as<{class_name}>();')
        self.gen_definition_body(genclass, lines, self._arguments, [None] * len(self._arguments), not self.check_argument_type,
                                 not self.check_argument_type)

        return '\n    '.join(lines)

    def gen_definition_body(self, genclass, lines, not_overloaded_args, gen_type_check_args, exact_cast, check_args=False, optional_check=False):
        bodylines = []
        args = [(i, j) for i, j in enumerate(not_overloaded_args) if j]
        args_set = {i for i, j in args}
        argdeclare = [j.gen_declare(self, f'obj{i}', f'arg{i}', exact_cast, optional_check) for i, j in args]
        self_type_checks = []
        if self._self_argument:
            if not self._self_argument.type_compare(f'sp<{genclass.binding_classname}>', f'{genclass.binding_classname}&', 'Box'):
                self_type_checks.append(f'unpacked.isInstance<{acg.get_shared_ptr_type(self._self_argument.accept_type)}>()')
        self._gen_convert_args_code(bodylines, argdeclare, optional_check)

        if check_args and args:
            bodylines.append("if(%s) %s;" % (' || '.join([f'!obj{i}' for i, j in args]), self.err_return_value))

        r = acg.strip_key_words(self._return_type, ['virtual', 'const', '&'])
        argtypes = [' '.join(i.gen_declare(self, 't', 't').split('=')[0].strip().split()[:-1]) for i in self._arguments]
        argvalues = ', '.join(gen_method_call_arg(f'obj{i}.value()' if optional_check and i in args_set else f'obj{i}', j, argtypes[i]) for i, j in enumerate(self._arguments))
        callstatement = self._gen_calling_statement(genclass, argvalues)
        py_return = self.gen_py_return()
        pyret = ['return 0;'] if py_return == 'int' else self.gen_return_statement(r, py_return)
        if r != 'void' and r:
            callstatement = '%s ret = %s' % (acg.strip_key_words(self._return_type, ['virtual']), callstatement)
        calling_lines = [callstatement] + pyret
        type_checks = [(i, j.gen_type_check('t', False)) for i, j in enumerate(gen_type_check_args) if j]
        nullptr_check = [f'(obj{i} || {i} >= argc)' for i, j in type_checks if not j]
        if optional_check:
            nullptr_check.extend(f'obj{i}' for i in args_set)
        if self_type_checks or nullptr_check:
            if nullptr_check:
                lines.insert(0, '[[maybe_unused]] const Py_ssize_t argc = %s;' % self.gen_py_argc())
            nullptr_check = self_type_checks + nullptr_check
            calling_lines = ['if(%s)' % ' && '.join(nullptr_check), '{'] + [INDENT + i for i in calling_lines] + ['}']
        lines.extend(bodylines + self._gen_calling_body(genclass, calling_lines))

    @staticmethod
    def split(repl):
        name = repl[0].split()[-1]
        rs = repl[0].split()[:-1]
        return_type = ' '.join(i for i in rs if i not in('static', 'ARK_API'))
        # args = [i.strip() for i in repl[1].split(',') if i]
        args = [i.strip() for i in GenMethod._bracket_match_split(repl[1], '<(', '>)', ',') if i]
        return name, args, return_type, 'static' in rs

    @staticmethod
    def _bracket_match_split(content, bracket_open, bracket_close, splitter):
        balance = 0
        results = []
        split_start = 0
        for i, j in enumerate(content):
            if j in bracket_open:
                balance += 1
            elif j in bracket_close:
                assert balance > 0
                balance -= 1
            elif j == splitter and balance == 0:
                results.append(content[split_start: i])
                split_start = i + 1
        assert balance == 0
        if split_start < len(content):
            results.append(content[split_start:])
        return results

    def overload(self, m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(type(self))(m1, m2)


class GenGetPropMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    def _gen_convert_args_code(self, lines, argdeclare, optional_check=False):
        if argdeclare:
            arg0 = self._arguments[0]
            meta = GenArgumentMeta('PyObject*', arg0.accept_type, 'O')
            ga = GenArgument(0, arg0.accept_type, arg0.default_value, meta, str(arg0))
            lines.append(ga.gen_declare(self, 'obj0', 'arg0'))

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* arg0'

    def gen_py_argc(self):
        return 1

    def _gen_calling_body(self, genclass, calling_lines):
        return ['PyObject* attr = PyObject_GenericGetAttr(reinterpret_cast<PyObject*>(self), arg0);',
                'if(attr)',
                INDENT + 'return attr;',
                'if(!PythonExtension::instance().exceptErr(PyExc_AttributeError))',
                f'{INDENT}{PY_RETURN_NONE};'
                '\n',
                'try {'] + [INDENT + i for i in calling_lines] + [
                '} catch(const std::exception& ex) {',
                   INDENT + 'PyErr_SetString(PyExc_AttributeError, Strings::sprintf("\'%s\' object has no attribute \'%%s\'. \\nCaused by: %%s", obj0.c_str(), ex.what()).c_str());' % genclass.classname,
                   INDENT + 'return nullptr;',
                '}'
        ]


class GenSetPropMethod(GenMethod):
    def __init__(self, name, args, return_type):
        GenMethod.__init__(self, name, args, return_type)

    def gen_py_return(self):
        return 'int'

    @property
    def err_return_value(self):
        return 'return -1'

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    def gen_py_arguments(self):
        return 'Instance* self, PyObject* arg0, PyObject* arg1'

    def gen_py_argc(self):
        return 2


class GenOperatorMethod(GenMethod):
    def __init__(self, name, args, return_type: str, operator: str, is_static: bool):
        GenMethod.__init__(self, name, args, return_type, is_static)
        is_call_operator = operator == 'call'
        self._self_argument = None if self._is_static or is_call_operator else self._arguments and self._arguments[0]
        if not is_call_operator:
            self._arguments = self._arguments if self._is_static else self._arguments[1:]
        self._operator = operator
        self._return_bool = self._return_type == 'bool'

    def gen_py_return(self):
        return 'int32_t' if self._return_bool else 'PyObject*'

    def need_unpack_statement(self):
        return not self._is_static and super().need_unpack_statement()

    @property
    def check_argument_type(self):
        return self._operator not in ('&&', '||')

    def gen_py_arguments(self):
        if self._operator == 'call':
            assert not self._is_static
            return f'Instance* self, PyObject* args, PyObject* kws'
        arglen = len(self._arguments)
        args = ['PyObject* arg%d' % i for i in range(arglen)]
        if self._is_static:
            return ', '.join(args)
        return ', '.join(['Instance* self'] + args)

    def gen_py_argc(self):
        return len(self._arguments)

    def _gen_calling_statement(self, genclass, argnames):
        if self._is_static:
            return '%s::%s(%s);' % (genclass.classname, self._name, argnames)
        return super()._gen_calling_statement(genclass, argnames)

    def gen_return_statement(self, return_type, py_return):
        if self._operator in ('+=', '-=', '*=', '/='):
            return ['Py_INCREF(self);', 'return reinterpret_cast<PyObject*>(self);']
        return super().gen_return_statement(return_type, py_return)

    def _gen_parse_tuple_code(self, lines, declares, args):
        if self._operator == 'call':
            return super()._gen_parse_tuple_code(lines, declares, args)

    @property
    def err_return_value(self):
        return 'return 0' if self._return_bool else 'Py_RETURN_NOTIMPLEMENTED'

    @property
    def operator(self):
        return self._operator

    def overload(self, m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            assert m1.operator == m2.operator and m1.is_static == m2.is_static
            return create_overloaded_method_type(GenOperatorMethod, operator=m1.operator, is_static=m1.is_static)(m1, m2)


class GenSubscribableMethod(GenMethod):
    def __init__(self, name, args, return_type, operator, is_static):
        self._operator = operator
        self._is_static = is_static
        self._is_len_func = operator == 'len'
        self._is_set_func = len(args) == 3
        super().__init__(name, args[1:] if self._is_static else args, return_type, is_static)

    @property
    def operator(self):
        return self._operator

    @property
    def err_return_value(self):
        return 'return -1' if len(self._arguments) == 2 else super().err_return_value

    def _gen_parse_tuple_code(self, lines, declares, args):
        pass

    def gen_py_argc(self):
        return len(self._arguments)

    def gen_py_return(self):
        if self._is_len_func:
            return 'Py_ssize_t'
        return 'int32_t' if len(self._arguments) == 2 else super().gen_py_return()

    def overload(self, m1, m2):
        try:
            m1.add_overloaded_method(m2)
            return m1
        except AttributeError:
            return create_overloaded_method_type(type(self), is_static=m1.is_static, operator=m1.operator)(m1, m2)

    def _gen_calling_statement(self, genclass, argnames) -> str:
        calling_statement = '%s::%s(%s);' % (genclass.classname, self._name, f'unpacked{", " if argnames else ""}{argnames}') if self._is_static else super()._gen_calling_statement(genclass, argnames)
        if self._is_set_func:
            for i in genclass.subscribe_methods():
                if i.operator == 'del':
                    argnames = argnames.split(',')[0].strip()
                    return f'if(arg1)\n{INDENT * 2}{calling_statement}\n{INDENT}else\n{INDENT * 2}{i._gen_calling_statement(genclass, argnames)}'
        return calling_statement


class GenMappingMethod(GenSubscribableMethod):
    def __init__(self, name: str, args, return_type: str, operator: str, is_static: bool):
        super().__init__(name, args, return_type, operator, is_static)

    def _gen_convert_args_code(self, lines, argdeclare, optional_check=False):
        if argdeclare and not self._is_len_func:
            for i, j in enumerate(self._arguments):
                meta = GenArgumentMeta('PyObject*', j.accept_type, 'O')
                ga = GenArgument(j.index, j.accept_type, j.default_value, meta, str(j))
                lines.append(ga.gen_declare(self, f'obj{i}', f'arg{i}', False, optional_check))

    def gen_py_arguments(self):
        self_arg = f'Instance* self'
        if self._is_len_func:
            return self_arg

        argc = self.gen_py_argc()
        return f'{self_arg}, {", ".join("PyObject* arg%d" % i for i in range(argc))}'


class GenSequenceMethod(GenSubscribableMethod):
    def __init__(self, name, args, return_type, operator, is_static):
        super().__init__(name, args, return_type, operator, is_static)

    def _gen_convert_args_code(self, lines, argdeclare, optional_check=False):
        if argdeclare and not self._is_len_func:
            arg0 = self._arguments[0]
            lines.append(f'{arg0.accept_type} obj0 = static_cast<{arg0.accept_type}>(arg0);')
            for i, j in enumerate(self._arguments[1:]):
                meta = GenArgumentMeta('PyObject*', j.accept_type, 'O')
                ga = GenArgument(j.index, j.accept_type, j.default_value, meta, str(j))
                lines.append(ga.gen_declare(self, f'obj{i + 1}', f'arg{i + 1}', False, optional_check))

    def gen_py_arguments(self):
        self_arg = f'Instance* self'
        if self._is_len_func:
            return self_arg

        argc = self.gen_py_argc()
        argc_types = ['Py_ssize_t', 'PyObject*']
        return f'{self_arg}, {", ".join("%s arg%d" % (argc_types[i], i) for i in range(argc))}'

    def gen_return_statement(self, return_type, py_return):
        return_statement = super().gen_return_statement(return_type, py_return)
        if not self._is_len_func and not self._is_set_func:
            index_check = [
                'if(!ret)',
                '{',
                '    PyBridge::setStopIterationErrString("");',
                '    return nullptr;',
                '}'
            ]
            return index_check + return_statement
        return return_statement


TP_AS_MAPPING_TEMPLATE = [
    'static PyMappingMethods ${py_class_name}_tp_as_mapping = {',
    '    (lenfunc) ${mp_length},        /* lenfunc mp_length;                  */',
    '    (binaryfunc) ${mp_subscribe}, /* binaryfunc mp_subscript; */',
    '    (objobjargproc) ${mp_ass_subscript}, /* objobjargproc mp_ass_subscript; */',
    '};'
]


TP_AS_MAPPING_TEMPLATE_OPERATOR = {
    'len': 'mp_length',
    'get': 'mp_subscribe',
    'set': 'mp_ass_subscript'
}


TP_AS_SEQUENCE_TEMPLATE = [
    'static PySequenceMethods ${py_class_name}_tp_as_sequence = {',
    '    (lenfunc) ${sq_length},               /* lenfunc sq_length;        */',
    '    (binaryfunc) ${sq_concat},            /* binaryfunc sq_concat;     */',
    '    (ssizeargfunc) ${sq_repeat},          /* ssizeargfunc sq_repeat;   */',
    '    (ssizeargfunc) ${sq_item},            /* ssizeargfunc sq_item;     */',
    '    nullptr,               /* was_sq_slice */',
    '    (ssizeobjargproc) ${sq_ass_item},               /* ssizeobjargproc sq_ass_item;*/',
    '    nullptr,               /* was_sq_ass_slice */',
    '    nullptr,               /* sq_contains */',
    '    nullptr,               /* sq_inplace_concat */',
    '    nullptr,               /* sq_inplace_repeat */',
    '};'
]


TP_AS_SEQUENCE_TEMPLATE_OPERATOR = {
    'len': 'sq_length',
    '+': 'sq_concat',
    '*': 'sq_repeat',
    'get': 'sq_item',
    'set': 'sq_ass_item'
}


def gen_template_defs(py_class_name: str, methods, code_templates: list[str], operator_templates: dict[str, str]):
    code_lines = []
    if methods:
        args = {'py_class_name': py_class_name}
        methods_dict = dict((i.operator, f'{py_class_name}::{i.name}_r') for i in methods)
        args.update((v, methods_dict[k] if k in methods_dict else 'nullptr') for k, v in operator_templates.items())
        code_lines.extend(acg.format(i, **args) for i in code_templates)
    return code_lines


def gen_as_number_operator_defs(genclass):
    as_number_operator_methods = [i for i in genclass.operator_methods() if i.operator in TP_AS_NUMBER_TEMPLATE_OPERATOR]
    return gen_template_defs(genclass.py_class_name, as_number_operator_methods, TP_AS_NUMBER_TEMPLATE, TP_AS_NUMBER_TEMPLATE_OPERATOR)


def gen_as_mapping_defs(genclass):
    return gen_template_defs(genclass.py_class_name, genclass.subscribe_methods(), TP_AS_MAPPING_TEMPLATE, TP_AS_MAPPING_TEMPLATE_OPERATOR)


def gen_as_sequence_defs(genclass):
    return gen_template_defs(genclass.py_class_name, genclass.sequence_methods(), TP_AS_SEQUENCE_TEMPLATE, TP_AS_SEQUENCE_TEMPLATE_OPERATOR)
