#!/opt/miniconda3/bin/python
import os
import sys

for line in sys.stdin:
    msg = line.lstrip().split(' ', 7)
    sys.stdout.write((msg[-1].replace(']', '')))