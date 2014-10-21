import numpy as np

class Point: 
    def __init__(self, x, y, z = 0, color = 1): 
        self.v = np.array([[x],[y],[z]])
        self.color = color
