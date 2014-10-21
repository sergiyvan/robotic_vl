import numpy as np
from math import acos

def getDistance(lines_1, lines_2):
    unit = np.array([[1],[0]]) # a unit vector for dircetion calculation

    # bin the lines according to their direction (17 bins)
    bins = [[],[],[],[],[],[],
            [],[],[],[],[],[],
            [],[],[],[],[]]
    for line in lines_1:
        bins[round(10 * acos(np.sum(np.abs(line.direction) * unit)))].append(line)

    distance = 0
    for line in lines_2:
        bin_to_check = bins[round(10 * acos(np.sum(np.abs(line.direction) * unit)))]
        lines_distance = 0
        for line_to_check in bin_to_check:
            d = line.getDistance(line_to_check)
            if lines_distance > d:
                d = lines_distance
        distance += lines_distance
    return distance

