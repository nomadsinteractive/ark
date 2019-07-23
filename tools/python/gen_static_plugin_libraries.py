#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import sys


def main():
    opts, args = getopt.getopt(sys.argv[1:], 'c:n:o:s:b:t:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in []) or len(args) == 0:
        print('Usage: %s src_paths...' % sys.argv[0])
        sys.exit(0)

    output_file = args[-1]
    plugin_names = args[:-1]

    declaration = 'std::map<ark::String, ark::PluginManager::PluginInitializer> _ark_static_plugin_initializers'

    lines = ['#include <map>\n\n', '#include "core/base/plugin_manager.h"\n\n', '\nextern %s;\n\n' % declaration]
    lines.extend('extern "C" ark::Plugin* __%s_initialize__(ark::Ark&);\n' % i.replace('-', '_') for i in plugin_names)
    lines.append('\n\n')
    lines.append('%s = {\n    ' % declaration)
    lines.append(',\n    '.join('{"%s", __%s_initialize__}' % (i, i.replace('-', '_')) for i in plugin_names))
    lines.append('\n};\n\n')
    with open(output_file, 'wt') as fp:
        fp.writelines(lines)


if __name__ == '__main__':
    main()
