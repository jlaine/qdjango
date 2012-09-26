#!/usr/bin/python

import os

root = os.path.join(os.path.dirname(__file__), '..')
prog = os.path.join(os.path.dirname(__file__), 'qdjango-tests')

path = []
for component in ['db', 'http', 'script']:
    path.append(os.path.join(root, 'src', component))

os.system("LD_LIBRARY_PATH=%s %s" % (':'.join(path), prog))
