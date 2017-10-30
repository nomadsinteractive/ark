#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import re
import os

_INTEGER_PATTERN = re.compile(r'\d+')
FIRST_CAP_RE = re.compile('(.)([A-Z][a-z]+)')
ALL_CAP_RE = re.compile('([a-z0-9])([A-Z])')


def toCamelName(namespaces):
    return ''.join([i[0].upper() + i[1:] for i in namespaces if i])


def camelName(namespaces):
    name = ''.join([i[0].upper() + i[1:] for i in namespaces if i])
    return name[0].lower() + name[1:] if name else ''


def findMainClass(content):
    m = re.search(r'class\s+(?:[\w\d]+\s+)*?([\w\d]+)(?:\s+final\s*)?(?:\s*:[^{]+|\s*)\{', content)
    return m.group(1) if m else None


def camel_case_to_snake_case(name):
    s1 = FIRST_CAP_RE.sub(r'\1_\2', name)
    return ALL_CAP_RE.sub(r'\1_\2', s1).lower()


def format(str, *args, **kwargs):
    def repl(m):
        v = m.group(1)
        if _INTEGER_PATTERN.match(v):
            return args[int(v)]
        return kwargs[v]
    return re.sub(r'\$\{([^}]+)\}', repl, str)


def matchHeaderPatterns(paths, *args):
    for src_code_path in paths:
        if os.path.isfile(src_code_path):
            _process_header(src_code_path, *args)
        elif os.path.isdir(src_code_path):
            for dirname, dirnames, filenames in os.walk(src_code_path):
                for i in [j for j in filenames if j.endswith('.h')]:
                    filename = os.path.join(dirname, i)
                    _process_header(filename, *args)


def _process_header(filename, *args):
    with open(filename, 'rt') as fp:
        filename = filename.lstrip(r'.\/').replace('\\', '/')
        content = fp.read()
        main_class = findMainClass(content)
        if main_class:
            for j in args:
                for k in j['pattern'].findall(content):
                    j['callback'](filename, content, main_class, k)



def wrapByNamespaces(namespaces, source):
    return '\n'.join(['namespace %s {' % i for i in namespaces]) + '\n' + source + '\n\n' + '\n'.join(['}'] * len(namespaces))


def stripKeywords(statement, keywords):
    return ' '.join(i for i in (statement.split() if type(statement) is str else statement) if i and i not in keywords)


def getSharedPtrType(typename):
    t = stripKeywords(typename, ['const']).replace('&', '').strip()
    return t[3:-1] if t.startswith('sp<') and t.endswith('>') else t


def get_argument_type(statement):
    s = statement.split()
    if len(s) > 1:
        s = s[:-1]
    return ' '.join(s)


def get_argument_name(statement):
    s = statement.split()
    return s[-1]


def typeCompare(t1, t2):
    t1 = stripKeywords(t1, ['static', 'const', 'virtual']).replace('&', '').strip()
    t2 = stripKeywords(t2, ['static', 'const', 'virtual']).replace('&', '').strip()
    l = min(len(t1), len(t2))
    return t1[:l] == t2[:l]


def write_to_unit(unit_name, header, source):
    names = re.sub(r'[^\d\w_]', '_', unit_name or 'stdout')
    macro = names.upper() + '_H_'
    write_to_file(unit_name and unit_name + '.h', '#ifndef %s\n#define %s\n\n%s\n\n#endif' %(macro, macro, header))
    write_to_file(unit_name and unit_name + '.cpp', source)


def write_to_file(filename, content):
    if filename is None:
        print(content)
    else:
        dirname = os.path.dirname(filename)
        if dirname and not os.path.isdir(dirname):
            os.makedirs(dirname)
        with open(filename, 'wt') as fp:
            fp.write(content)
