#!/usr/bin/env python

""""
This script performs some analytic stuff.

Pass an integer to only rerun the tasks if the current products
are older than the passed integer interpreted as hours.

In order to add a new task extend task_list.

Author: Stefan Otte
"""


import os
import time
import sys


# combination of location to check and command to execute
task_list = [\
          'mkdir -p /tmp/.slocdata_fumanoids'
        , 'sloccount --datadir /tmp/.slocdata_fumanoids --wide --details src > build/sloccount.sc'
        , 'cppcheck --xml src/ 2> cppcheck-result.xml'
        ]

# max time difference for last task execution
max_diff = 0

# save execution time
le = 'build/.last_executed'


def last_executed():
    """ return the last execution time """
    try:
        with open(le) as f:
            t = f.readline()
            if t == '':
                return 0
            return int(float(t))
    except IOError:
        print 0
        return 0


def update_last_execution():
    """ write the execution time to file """
    with open(le, 'w') as f:
        f.write(str(time.time()))


if __name__ == '__main__':
    if len(sys.argv) == 2:
        max_diff = int(sys.argv[1])

    now = int(time.time()) / 3600
    last = int(last_executed()) / 3600
    diff = now - last

    #print 'now:  ', now
    #print 'last: ', last
    #print 'diff: ', diff

    if diff >= max_diff:
        for task in task_list:
            print '=' * 78
            print 'executing task for ', task
            os.system(task)
        update_last_execution()
