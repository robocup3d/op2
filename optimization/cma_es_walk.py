#!/usr/bin/env python3
import cma
import atexit
from time import sleep
from run_params import *
from os import listdir
import pickle
import sys
import json
import pymysql

db = pymysql.connect('192.168.1.163', user='root', password='robocup3d', db='optimization')
cursor = db.cursor()

WALK_RESULT_FILE_NAME = 'walkout'
PARAMS_RECORD_FILE_NAME = 'params-record.json'
PARAMS_RECORD_FILE = 'walk-record.txt'
std_list, params_names = read_params()
if PARAMS_RECORD_FILE_NAME in listdir('.'):
    record = json.load(open(PARAMS_RECORD_FILE_NAME, 'r'))
    print('load pickle object from ', PARAMS_RECORD_FILE_NAME)
else:
    record = {
        'robot type': ROBOT_TYPE,
        "param_names": params_names,
        "record": []
    }


def eva(ind):
    # global record
    paramList = [e * ind[i] for i, e in enumerate(std_list)]
    time_cost = run(paramList, ROBOT_TYPE)
    # record['record'].append(
    #     {
    #         'parameters': paramList,
    #         'walk time_cost': time_cost
    #     }
    # )
    # json.dump(record, open(PARAMS_RECORD_FILE_NAME, 'w'))
    with open(PARAMS_RECORD_FILE, 'a') as f:
        print(*paramList, time_cost, sep=',', file=f)
    # dump2json(PARAMS_RECORD_FILE_NAME, paramList, params_name, time_cost)
    print('walk time cost: ', time_cost)
    cursor.execute(
        "insert into walk_straight values (" +
        str(paramList)[1:-1] + ',' +
        str(time_cost) + ")"
    )
    db.commit()
    print("data base updated!")
    return time_cost ** 2


def store_data():
    # print('receive terminated signal, exit...')
    pickle.dump(es, open(pkl, 'wb'))
    json.dump(record, open(PARAMS_RECORD_FILE_NAME, 'w'), indent=4, separators=(',', ': '))
    print('data has been dumped into ', pkl)


if __name__ == '__main__':

    pkl = '_saved-cma-object.pkl'
    if pkl in listdir('.'):
        es = pickle.load(open(pkl, 'rb'))
        print('load pickle object from ', pkl)
        sleep(3)
    else:
        es = cma.CMAEvolutionStrategy(len(std_list) * [1.0], 0.05,
                                      {'maxiter': 300, 'popsize': 100, 'bounds': [0.5, 1.5]})

    atexit.register(store_data)
    # store_data()
    i = 0
    try:
        while True:
            solutions = es.ask()
            es.tell(solutions, [eva(s) for s in solutions])
            i += 1
            store_data()
            # pickle.dump(es, open(pkl, 'wb'))
    except KeyboardInterrupt as e:
        print('receive terminated signal, exit...')
        store_data()
        sys.exit(1)
