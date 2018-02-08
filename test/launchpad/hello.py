from time import time, ctime
import sys
import os
import ark

print('Hello world, today is ', ctime(time()))
print(dir(ark))

from _frozen_importlib import ModuleSpec
from _frozen_importlib import ArkModuleFinder
from _frozen_importlib import ArkModuleLoader
sys.meta_path.append(ArkModuleFinder(ark, ModuleSpec, ArkModuleLoader, ['asset:///test.egg', os.path.join(os.getcwd(), 'libs')]))

import test_module
print(test_module)

import my_test
print(my_test)

from my_test import my_module as m1
print(m1)

from edis import ascii
print(ascii)

from my_lib_test import my_module as m2
print(m2)


if not ark.Boolean(True):
    print('Error loading Boolean(True)')
if ark.Boolean(False):
    print('Error loading Boolean(False)')

if float(ark.Numeric(0)) != 0:
    print('Error loading Numeric(0)')
if float(ark.Numeric(1) + ark.Numeric(2)) != 3:
    print('Error loading Numeric(1) + Numeric(2)')

if int(ark.Integer(0)) != 0:
    print('Error loading Integer(0)')
if int(ark.Integer(1) + ark.Integer(2)) != 3:
    print('Error loading Integer(1) + Integer(2)')
