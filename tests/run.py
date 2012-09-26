#!/usr/bin/python

import os

root = os.path.join(os.path.dirname(__file__), '..')

# set library path
path = []
for component in ['db', 'http', 'script']:
    path.append(os.path.join(root, 'src', component))
os.environ['LD_LIBRARY_PATH'] = ':'.join(path)

# run tests
for component in ['db', 'http']:
    prog = os.path.join(os.path.dirname(__file__), component, 'qdjango-%s-tests' % component)
    os.system(prog)
