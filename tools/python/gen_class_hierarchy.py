#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import re
import sys

import acg
from acg import HeaderPattern

CLASS_PATTERN = re.compile(r'([\w<():=,>\s]+)class\s+(?:ARK_API\s+)?(\w+)(?:\s+final)?\s+(:[^{;]+)?[{;]')

INDENT = '\n    '


CORE_INTERFACES = ('Numeric', 'Integer', 'Resource', 'EventListener', 'Uploader', 'Renderable', 'Renderer', 'Notifier', 'ModelLoader',
                   'Boolean', 'Runnable', 'Vec2', 'Vec3', 'Vec4', 'Mat3', 'Mat4', 'Debris', 'Wirable', 'CollisionCallback', 'Wirable::Niche')


class GenClass:
    def __init__(self, name, implements, filename):
        self._name = name
        self._implements = implements
        self._filename = filename

    def include(self):
        return '#include "%s"' % self._filename

    def add_class(self):
        implements = ', '.join(self._implements)
        return 'classManager.addClass<%s%s>("%s");' % (self._name, ', %s' % implements if implements else '', self._name)


def search_for_classes(paths):
    result = []

    def match_class(filename, content, main_class, x):
        class_pre, class_name, impls = x
        if class_name == main_class and not class_name.startswith('_') and 'template<' not in class_pre:
            if impls.startswith(':'):
                implements = [k for k in [j[6:].strip() for j in [i.strip() for i in impls[1:].split(',')] if j.startswith('public')] if not k.startswith('Class<')]
            else:
                implements = []
            if [i for i in implements if i in CORE_INTERFACES]:
                result.append(GenClass(class_name, implements, filename))

    acg.match_header_patterns(paths, True, HeaderPattern(CLASS_PATTERN, match_class))
    return result


def generate_bootstrap_func(func_name, classinfos):

    interfaces = ['classManager.addClass<%s>("%s");' % (i, i) for i in CORE_INTERFACES]

    return '''void __ark_bootstrap_%s_class_hierarchy__()
{
    ark::ClassManager& classManager = ark::ClassManager::instance();

    %s

    %s
}
''' % (func_name, INDENT.join(interfaces), INDENT.join([i.add_class() for i in classinfos]))


def main():
    opts, args = getopt.getopt(sys.argv[1:], 'n:o:s:b:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in ['n']) or len(args) == 0:
        print('Usage: %s -n name [-o output_file] [-s namespace] src_paths...' % sys.argv[0])
        sys.exit(0)

    name = params['n']
    output_file = params['o'] if 'o' in params else None
    namespace = params['s'] if 's' in params else 'ark'

    result = search_for_classes(args)

    includes = '\n'.join(i.include() for i in result)
    src = '''
#include "core/ark.h"
#include "core/base/class_manager.h"
#include "core/impl/variable/variable_wrapper.h"

%s

using namespace %s;

%s

''' % (includes, namespace, generate_bootstrap_func(name, result))
    acg.write_to_file(output_file, src)


if __name__ == '__main__':
    main()
