#!/usr/bin/env python
#coding=utf-8
import os


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
    command = './sample_start-optimization.sh {0} {1} walkout'.format(ROBOT_TYPE, absPath)
    os.system(command)

    file = open('walkout')
    lines = file.readlines()
    file.close()
    distances = lines[-7:]
    
    return sum([float(_[:-1]) for _ in distances])/len(distances)


def readParams(fn='../paramfiles/optimizing.txt'):
    with open(fn) as f:
        lines = f.readlines()
        params = [float(_) for _ in [i[:-1].split('\t')[-1] for i in lines]]
    return params


def main():
    paramList = readParams()
    print(paramList)
    absPath = os.path.abspath('..')+'/paramfiles/optimizing.txt'
    #writeToFile(paramList, absPath)
    rst = run(paramList, ROBOT_TYPE, absPath)
    print('running average distance :', rst)


if __name__ == '__main__':
    main()
