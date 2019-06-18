#!/usr/bin/python
import atexit
import os

def wrt():
    os.system('touch test.log')


atexit.register(wrt)
while True:
    pass
