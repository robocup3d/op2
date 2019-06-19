from cma_es_walk import *


cursor.execute(
    "select * from walk_straight order by time_cost asc limit 5"
)
res = cursor.fetchall()

for _ in res:
    score = run(_[:-1], ROBOT_TYPE)
    print('running average time cost:', score+85)
