#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
import sys
import re
import acg

CLASS_PATTERN = re.compile(r'\[\[core::class\]\]\s+class\s+(?:ARK_API\s+)?([\w\d_]+)(?:\s+final)?\s*(:?[^{]+){')

INDENT = '\n    '


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
        class_name = x[0]
        if x[1].startswith(':'):
            implements = [k for k in [j[6:].strip() for j in [i.strip() for i in x[1][1:].split(',')] if j.startswith('public')] if not k.startswith('Class<')]
        else:
            implements = []
        result.append(GenClass(class_name, implements, filename))

    acg.matchHeaderPatterns(paths,
                            {'pattern': CLASS_PATTERN, 'callback': match_class},
                            )
    return result


def generate_bootstrap_func(func_name, classinfos):
    return '''void __ark_bootstrap_%s_class_hierarchy__()
{
    ark::ClassManager& classManager = ark::ClassManager::instance();

    %s

    classManager.updateHierarchy();
}
''' % (func_name, INDENT.join([i.add_class() for i in classinfos]))


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
    file_path = None
    if output_file:
        file_dir, file_path = os.path.split(output_file)

    includes = '\n'.join(i.include() for i in result)
    src = '''
#include "core/ark.h"
#include "core/base/class_manager.h"

%s

using namespace %s;

%s

''' % (includes, namespace, generate_bootstrap_func(name, result))
    acg.write_to_file(output_file, src)


if __name__ == '__main__':
    main()
