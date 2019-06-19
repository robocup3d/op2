#!/usr/bin/env python3
# coding=utf-8
import os
import json

ROBOT_TYPE = 2


def write2file(param_list, file_path):
    with open(file_path, 'r+') as file:
        param_lines = file.readlines()
        param_names = [i.split('\t')[0] for i in param_lines]

    new_lines = [
        e + '\t' + str(param_list[i]) + '\n'
        for i, e in enumerate(param_names)
    ]
    with open(file_path, 'w+') as file:
        file.writelines(new_lines)


def run(param_list, robot_type, abs_path='../paramfiles/optimizing.txt'):
    write2file(param_list, abs_path)
    command = './sample_start-optimization.sh {0} {1} walkout'.format(robot_type, abs_path)
    os.system(command)
    with open('walkout') as file:
        lines = file.readlines()[-1]
        try:

            # time_cost = [float(_.strip('\n')) for _ in lines]
            return float(lines.strip('\n'))
        except ValueError:
            return 1000


def read_params(fn='../paramfiles/original.txt'):
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
    param_list, params_name = read_params()
    # print(paramList)
    abs_path = os.path.abspath('..') + '/paramfiles/original.txt'
    # write2file(paramList, absPath)
    rst = run(param_list, ROBOT_TYPE, abs_path)
    print('go to target average time cost:', rst)


if __name__ == '__main__':
    main()
