#!/usr/bin/env python
#coding=utf-8
import os
from optim_libs import *


ROBOT_TYPE = 2


def writeToFile(paramList, filePath):
    file = open(filePath, 'r+')
    paramLines = file.readlines()
    file.close()
    paramNames = [i.split('\t')[0] for i in paramLines]

    newLines = []
    for i, e in enumerate(paramNames):
        newLines.append(e + '\t' + str(paramList[i]) + '\n')

    file = open(filePath, 'w+')
    file.writelines(newLines)
    file.close()


def run(paramList, roboType, absPath):
    writeToFile(paramList, '../paramfiles/optimizing.txt')
    command = './sample_start-optimization.sh {0} {1} out'.format(ROBOT_TYPE, absPath)
    os.system(command)

    file = open('./out')
    lines = file.readlines()
    file.close()
    distance, time, fit = lines[-1][:-1].split(',')
    
    return float(distance)


def main():
    paramList = find_max()
    print(paramList)
    absPath = os.path.abspath('..')+'/paramfiles/optimizing.txt'
    #writeToFile(paramList, absPath)
    rst = run(paramList, ROBOT_TYPE, absPath)
    print('kicking average distance :', rst)


if __name__ == '__main__':
    main()
