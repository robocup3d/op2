#!/usr/bin/python
from matplotlib import pyplot


if __name__ == '__main__':
    with open('walk-record.txt') as f:
        lines = f.readlines()
        score = [float(k) for k in [_[:-1].split(',')[-1] for _ in lines]]
    pyplot.plot(score, '.')
    pyplot.title('walk score as times')
    pyplot.xlabel('try times')
    pyplot.ylabel('walk distance')
    best = [max(score[:i+1]) for i, e in enumerate(score)]
    pyplot.plot(best, 'r')
    pyplot.savefig('scores.png')
    pyplot.show()

