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


b = _resource_loader.load(ark.Boolean, 'g14')
if not b:
    print('Error loading g15')
b.set(False)
if b:
    print('Error setting g15')
