#!/usr/bin/python3
from matplotlib import pyplot
with open('pose') as f:
    lines = f.readlines()[:-1]
    x = [float(_[:-1].split(',')[0]) for _ in lines]
    y = [float(_[:-1].split(',')[1]) for _ in lines]
    z = [float(_[:-1].split(',')[2]) for _ in lines]

pyplot.plot(x,label='x')
pyplot.plot(y,label='y')
pyplot.plot(z,label='z')
pyplot.legend()
pyplot.title('acceleration during walk')
pyplot.show()
