#!/usr/bin/python
import cma
import atexit
from time import sleep
from run_params import run
from os import listdir
import pickle
from optim_libs import *


std_list = get_std_list()


def eva(ind):
    
    paramList = []
    for x in range(len(ind)):
        paramList.append(ind[x] * std_list[x])
    distance = run(paramList, 2, '../paramfiles/optimizing.txt')
    with open(OUTPUT_FILE_NAME, 'a') as file:
        newLines = ','.join([str(_) for _ in paramList])
        newLines += ','+str(distance )+'\n' 
        file.writelines(newLines)
    print('distance fitness: ', distance)
    return distance


if __name__ == '__main__':
    pkl = '_saved-cma-object.pkl'
    if pkl in listdir('.'):
        es = pickle.load(open(pkl, 'rb'))
        print('load', pkl)
        sleep(3)
    else:
        es=cma.CMAEvolutionStrategy(len(std_list)*[1.0], 0.3, {'maxiter':300, 'popsize':150, 'bounds': [0.8, 1.2]})
    def store_data():
        print('receive terminated signal, exit...')
        pickle.dump(es, open(pkl, 'wb'))
        print('data has been dumped into ', pkl)
    atexit.register(store_data)
    i = int(0)
    try:
        while True:
            solutions = es.ask()
            es.tell(solutions, [eva(s) for s in solutions])
            i += 1
            if i % 10 == 0:
                pickle.dump(es, open(pkl, 'wb'))
    except Exception as e:
        store_data()
    
