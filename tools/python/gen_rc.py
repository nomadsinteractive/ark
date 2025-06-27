#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import os
import sys

import acg

if __name__ == '__main__':
    opts, args = getopt.getopt(sys.argv[1:], 'i:')
    params = dict((i.lstrip('-'), j) for i, j in opts)
    if any(i not in params for i in []) or len(args) != 1:
        print('Usage: %s -i iconfile output_rc_file' % sys.argv[0])
        sys.exit(-1)

    output_rc_file = args[0]
    icon_filepath = os.path.abspath(params['i']).replace('\\', r'\\')
    content = f'IDI_ICON1 ICON DISCARDABLE "{icon_filepath}"'
    acg.write_to_file(output_rc_file, content)
