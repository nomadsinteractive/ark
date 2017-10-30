#!/usr/bin/env python
# -*- coding: utf-8 -*-

import acg
import os
import sys
import getopt

if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], 'o:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in []) or len(args) == 0:
        print('Usage: %s [-o output_unit] bootstrap_funcs...' % sys.argv[0])
        sys.exit(-1)

    output_file = params['o'] if 'o' in params else None
    declares = '\n'.join('extern void %s();' % i for i in args)
    invokes = '\n    '.join('%s();' % i for i in args)
    file_directory, file_name = os.path.split(output_file or 'stdout')
    function_name = file_name
    header = 'void __%s__();' % function_name
    source = acg.format("""${declares}

void __${function_name}__()
{
    ${invokes}
}
""", function_name=function_name, declares=declares, invokes=invokes)
    acg.write_to_unit(output_file, header, source)
