import numpy as np
from math import sin, cos, pi

class Circle:
    def __init__(self, m, r):
        self.m = m
        self.r = r

    def get_points(self, scale = 1):

        points = list()
        r_vector = np.array([[self.r],[0],[0]])
        step = 2*pi*scale/180
        i = 0
        while i*step < 360:
            points.append(self.m.v + r_vector)
            rot = np.array([[-sin(i*step), cos(i*step), 0],
                    [cos(i*step), sin(i*step), 0],
                    [0, 0, 1]])
            r_vector = np.dot(rot, r_vector)
            i += 1

        return points

    def get_color(self):

        return self.m.color
