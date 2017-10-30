#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import re
import acg

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Usage: %s header_to_generate.h' % sys.argv[0])
        sys.exit(0)

    filename = sys.argv[1]
    packages = re.split(r'[\\/.]', filename)
    macro = '_'.join(packages).upper()
    with open(filename, 'wt') as fp:
        fp.write(acg.format('''#ifndef ARK_${macro}_
#define ARK_${macro}_

#endif
''', macro=macro))
