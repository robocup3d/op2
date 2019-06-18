#!/usr/bin/env python
#coding=utf-8
import os
import json


ROBOT_TYPE = 2


def writeToFile(paramList, filePath):
    with open(filePath, 'r+') as file:
        paramLines = file.readlines()
        paramNames = [i.split('\t')[0] for i in paramLines]

    newLines = [
        e + '\t' + str(paramList[i]) + '\n' 
        for i, e in enumerate(paramNames)
    ]
    with open(filePath, 'w+') as file:
        file.writelines(newLines)



def run(paramList, roboType, absPath):
    writeToFile(paramList, absPath)
    command = './sample_start-optimization.sh {0} {1} walkout'.format(ROBOT_TYPE, absPath)
    os.system(command)
    with open('walkout') as file:
        lines = file.readlines()[-1]
        try:
            # time_cost = [float(_.strip('\n')) for _ in lines]
            return float(lines.strip('\n'))
        except ValueError:
            return 1000


def readParams(fn='../paramfiles/original.txt'):
    with open(fn) as f:
        lines = f.readlines()
        params = [float(_) for _ in [i[:-1].split('\t')[-1] for i in lines]]
        params_name = [i[:-1].split('\t')[0] for i in lines]
    return params, params_name


def dump2json(fn, params, param_names, score):
    try:
        with open(fn, 'a+') as f:
            record = json.load(f)
            record['record'].append(
                {
                    'parameters': params,
                    'walk distance': score
                }
            )
        with open(fn, 'w') as f:
            json.dump(record, f, indent=4, separators=(',', ': '))
    except FileNotFoundError:
        record = {
          'robot type': ROBOT_TYPE,
          "param_names": param_names,
          "record": [
            {
            'parameters': params,
            'walk distance': score
            }
          ]
        }
        with open(fn, 'w') as f:
            json.dump(record, f, indent=4, separators=(',', ': '))
      
        
        
def main():
    paramList, params_name = readParams()
    #print(paramList)
    absPath = os.path.abspath('..')+'/paramfiles/original.txt'
    #writeToFile(paramList, absPath)
    rst = run(paramList, ROBOT_TYPE, absPath)
    print('go to target average time cost:', rst)


if __name__ == '__main__':
    main()
