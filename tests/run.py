#!/usr/bin/python

import os

components = ['db', 'http', 'script']
root = os.path.dirname(__file__)

# set library path
path = []
for component in components:
    path.append(os.path.join(root, '..', 'src', component))
os.environ['LD_LIBRARY_PATH'] = ':'.join(path)

# run tests
for component in components:
    component_root = os.path.join(root, component)
    for test in os.listdir(component_root):
        test_path = os.path.join(component_root, test)
        if os.path.isdir(test_path):
            prog = os.path.join(test_path, 'tst_' + test)
            os.system(prog)
