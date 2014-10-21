import numpy as np

class Line:
    def __init__(self, start, end):
        self.start = start
        self.end = end
        length = np.sqrt(np.sum(np.square(end.v - start.v)))
        self.direction = (end.v - start.v) / length

    def get_points(self, scale = 1):

        points = list()
        m = self.direction * scale
        points.append(self.start.v)

        """ If sum over start point is smaller than sum over endpoint, all points
        on the start point side of the line have a smaller sum.
        ergo, we compare against the sum with a factor f of 1 or -1"""
        if np.sum(self.start.v) > np.sum(self.end.v):
			f = -1
        else:
            f = 1

        i = 1
        while f * np.sum(self.start.v + i * m) <= f * np.sum(self.end.v):
            points.append(self.start.v + i*m)
            i+=1

        return points

    def get_color(self):
        return self.start.color

    """ This just returns the difference between the start/end points."""
    def get_distance(self, other_line):
        # Calculate the four possible distances
        ss = abs(self.start - other_line.start)
        se = abs(self.start - other_line.end)
        es = abs(self.end - other_line.start)
        ee = abs(self.end - other_line.end)
        return min(ss + ee, se + es)

