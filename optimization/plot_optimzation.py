import pymysql
from matplotlib import pyplot as plt
# connect to mysql service
try:
    db = pymysql.connect(
        '192.168.1.163',
        'root',
        'robocup3d',
        'optimization',
        charset='utf8mb4', port=3306
    )
    cursor = db.cursor()
    print('Connected to SQL server!')
except Exception as e:
    print('ERROR when connect to SQL server!')
    exit(1)


cursor.execute('select * from walk_straight')
res = cursor.fetchall()
time_costs = [
    _[-1] for _ in res
    if _[-1] < 50
]
plt.plot(time_costs, '.')
plt.show()
# print(res[-1])