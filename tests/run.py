#!/usr/bin/python

import getopt
import os
import platform
import subprocess
import sys

components = ['db', 'http', 'script']
root = os.path.dirname(__file__)
report_path = None

def usage():
    print "Usage: run.py [options]"

# parse options
try:
    opts, args = getopt.getopt(sys.argv[1:], 'hvx:')
except getopt.GetoptError, err:
    print err
    usage()
    sys.exit(2)

for opt, optarg in opts:
    if opt == '-h':
        usage()
        sys.exit()
    elif opt == '-v':
        os.environ['QDJANGO_DB_DEBUG'] = '1'
    elif opt == '-x':
        report_path = optarg
        if not os.path.exists(report_path):
            os.mkdir(report_path)

# set library path
path = []
for component in components:
    path.append(os.path.join(root, '..', 'src', component))
if platform.system() == 'Darwin':
    os.environ['DYLD_LIBRARY_PATH'] = ':'.join(path)
else:
    os.environ['LD_LIBRARY_PATH'] = ':'.join(path)

# run tests
for component in components:
    component_path = os.path.join(root, component)
    for test in os.listdir(component_path):
        test_path = os.path.join(component_path, test)
        if os.path.isdir(test_path):
            if platform.system() == 'Darwin':
                prog = os.path.join(test_path, 'tst_' + test + '.app', 'Contents', 'MacOS', 'tst_' + test)
            else:
                prog = os.path.join(test_path, 'tst_' + test)
            if not os.path.exists(prog):
                continue

            cmd = [ prog ]
            if report_path:
                cmd += ['-xunitxml', '-o',  os.path.join(report_path, test + '.xml') ]
            subprocess.call(cmd)
