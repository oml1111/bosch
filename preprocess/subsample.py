#!/usr/bin/python3
import sys
import random

if len(sys.argv) != 4:
    print("Invalid no. of arguments")

limit = int(sys.argv[3])

ouf = open(sys.argv[2], 'w')
with open(sys.argv[1], 'r') as inf:
    cnt = 0
    ouf.write(inf.readline())
    for line in inf:
        cnt += 1
        if cnt % 10000 == 0:
            print("Processing line %s" % cnt)
        if random.randint(1, limit) == 1:
            ouf.write(line)
