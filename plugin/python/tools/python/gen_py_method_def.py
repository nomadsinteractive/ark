#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
from os import path
import sys
import re

def tocamelname(namespaces):
    return ''.join([i[0].upper() + i[1:] for i in namespaces])


def declare_namespaces(namespaces, source):
    return '\n'.join(['namespace %s {' % i for i in namespaces]) + '\n\n' + source + '\n\n' +'\n'.join(['}'] * len(namespaces))


def output_tofile(content, filename, outputdir):
    if outputdir:
        filepath = path.join(outputdir, filename)
        dirpath = path.dirname(filepath)
        if not path.isdir(dirpath):
            os.makedirs(dirpath)
        with open(filepath, 'wt') as fp:
            fp.write(content)
    else:
        print(filename + ':')
        print(content)


_META_METHODS = ['absorb', 'expire', 'size']

class PyType:
    def __init__(self, filepath, classname, methods):
        self._filepath = filepath
        self._classname = classname
        self._methods = methods

    def include(self):
        return '#include "%s"' % self._filepath.replace('\\', '/')

    def define(self):
        if len([i for i in self._methods if not i[0].startswith('__')]) == 0:
            return ''
        return 'extern PyMethodDef PyArk%s_methods[];' % self._classname

    def declare(self):
        methoddeclare = ',\n    '.join('{"%s", %s, %s, nullptr}' % (i[0], self._get_method_definition(i), self._method_flags(i[0], i[2])) for i in self._methods if not i[0].startswith('__'))
        if not methoddeclare:
            return ''
        return 'PyMethodDef PyArk%s_methods[] = {\n    %s,\n    {NULL}\n};' % (self._classname, methoddeclare)

    def _method_flags(self, methodname, method):
        if methodname.startswith('__') and methodname.endswith('__'):
            return 'METH_O'

        flags = ['METH_VARARGS']
        args = [i.strip() for i in method.split(',') if i]
        if(len(args) > 1):
            flags.append('METH_KEYWORDS')
        return '|'.join(flags)

    def _get_method_definition(self, method):
        if method[0] not in _META_METHODS:
            return '(PyCFunction) PyArk%sType::%s' % (self._classname, method[0])
        return '(PyCFunction) (PyArkMetaType<%s>::%s)' % (self._classname, method[0])


if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], 'p:n:o:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in ['p']) or len(args) == 0:
        print('Usage: %s -p namespace -n name [-o output_dir] dir_paths...' % sys.argv[0])
        sys.exit(0)

    outputdir = params['o'] if 'o' in params else None
    namespaces = params['p'].replace('::', ':').split(':')

    pytypes = []
    for i in args:
        for j in os.listdir(i):
            m = re.match(r'py_ark_([_\w\d]+)type\.h', j)
            if m:
                classname = [k for k in m.group(1).split('_') if k]
                camelclassname = tocamelname(classname)
                filepath = path.join(i, j)
                with open(filepath, 'rt') as fp:
                    content = fp.read()
                m = re.search(r'class PyArk%sType : public PyArkType\s*{([\w\W]+)};' % camelclassname, content)
                if m:
                    methods = re.findall(r'static\s+PyObject\s*\*\s*([\w\d_]+)\s*\(Instance\s*\*\s*([\w\d_]+)((?:,\s*PyObject\s*\*\s*[\w\d_]+)*)\);', m.group(1))
                    if methods:
                        pytypes.append(PyType(filepath, camelclassname, methods))

    macro = '_'.join([i.upper() for i in namespaces] + ['PY_METHOD_DEF', 'H_'])
    methodsdeclare = '\n\n'.join([i.define() for i in pytypes])
    header = '''#ifndef {macro}
#define {macro}

#include <Python.h>

{methodsdeclare}

#endif'''.format(params['n'], macro=macro, methodsdeclare=declare_namespaces(namespaces, methodsdeclare))
    output_tofile(header, params['n'] + '.h', outputdir)

    includes = '\n'.join(i.include() for i in pytypes)
    source = '''
%s

#include "extension/py_ark_meta_type.h"

%s''' % (includes, declare_namespaces(namespaces, '\n\n'.join(i.declare() for i in pytypes)))
    output_tofile(source, params['n'] + '.cpp', outputdir)
