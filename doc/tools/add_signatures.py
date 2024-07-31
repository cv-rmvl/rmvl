"""
This code adds Python signatures to the docs.
"""
from __future__ import print_function
import sys
sys.dont_write_bytecode = True  # Don't generate .pyc files / __pycache__ directories

import json

import html_functions
import doxygen_scan

ROOT_DIR = sys.argv[1]
PYTHON_SIGNATURES_FILE = sys.argv[2]

python_signatures = dict()
with open(PYTHON_SIGNATURES_FILE, "rt") as f:
    python_signatures = json.load(f)
    print("Loaded Python signatures: %d" % len(python_signatures))

import xml.etree.ElementTree as ET
root = ET.parse(ROOT_DIR + 'rmvl.tag')
files_dict = {}

# constants and function from rmvl.tag
namespaces = root.findall("./compound[@kind='namespace']")
for ns in namespaces:
    ns_name = ns.find("./name").text
    doxygen_scan.scan_namespace_constants(ns, ns_name, files_dict)
    doxygen_scan.scan_namespace_functions(ns, ns_name, files_dict)

# class methods from rmvl.tag
classes = root.findall("./compound[@kind='class']")
for c in classes:
    c_name = c.find("./name").text
    file = c.find("./filename").text
    doxygen_scan.scan_class_methods(c, c_name, files_dict)

print('Doxygen files to scan: %s' % len(files_dict))

files_processed = 0
files_skipped = 0
symbols_processed = 0

for file in files_dict:
    anchor_list = files_dict[file]
    active_anchors = [a for a in anchor_list if a.cppname in python_signatures]
    if len(active_anchors) == 0: # no linked Python symbols
        files_skipped = files_skipped + 1
        continue

    active_anchors_dict = {a.anchor: a for a in active_anchors}
    if len(active_anchors_dict) != len(active_anchors):
        print('Duplicate entries detected: %s -> %s (%s)' % (len(active_anchors), len(active_anchors_dict), file))

    files_processed = files_processed + 1
    symbols_processed = symbols_processed + len(active_anchors_dict)
    html_functions.insert_python_signatures(python_signatures, active_anchors_dict, ROOT_DIR + file)

print('Done (processed files %d, symbols %d, skipped %d files)' % (files_processed, symbols_processed, files_skipped))
