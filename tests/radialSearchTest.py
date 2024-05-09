import math
import time
import os

myMap = []
mapSize = 50

# Staying in the center
me = [int(mapSize / 2), int(mapSize / 2)]
mySight = 20


for i in range(mapSize):
    thisRow = []
    for j in range(mapSize):
        thisRow.append("-")
    myMap.append(thisRow)

myMap[me[0]][me[1]] = "0"


def printMap():
    for i in range(mapSize):
        for j in range(mapSize):
            print(f"{myMap[i][j]} ", end="")
        print("")


printMap()
time.sleep(1 / 4)

# Searching
for r in range(1, mySight + 1):
    theta = 0
    dtheta = 1 / ((2) * r)
    while theta < (2 * math.pi):
        x = r * math.cos(theta)
        y = r * math.sin(theta)

        myMap[me[0] + round(x)][me[1] + round(y)] = "X"

        theta += dtheta
    printMap()
    time.sleep(1 / 4)
    os.popen("clear")
