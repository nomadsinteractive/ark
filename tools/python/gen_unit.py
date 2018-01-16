#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import re
import acg
from os import path

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Usage: %s unit_to_generate(.h|.cpp)' % sys.argv[0])
        sys.exit(0)

    hfilename = sys.argv[1] + '.h'
    packages = re.split(r'[\\/.]', hfilename)
    macro = '_'.join(packages).upper()
    with open(hfilename, 'wt') as fp:
        fp.write(acg.format('''#ifndef ARK_${macro}_
#define ARK_${macro}_

#endif
''', macro=macro))
    if not path.isfile(sys.argv[1] + '.cpp'):
        with open(sys.argv[1] + '.cpp', 'wt') as fp:
            fp.write('')
