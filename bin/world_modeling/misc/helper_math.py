from __future__ import division
from pylab import *

import numpy as np
#import scipy as sp
from scipy import stats


def normalize_radians(data):
    """
    Reduce angle to [-pi..+pi[
    Return the result.

    >>> normalize_radians(0.0)
    0.0
    >>> normalize_radians(np.pi)
    -3.141592653589793
    >>> normalize_radians(-np.pi)
    -3.141592653589793
    >>> normalize_radians(np.pi/2)
    1.5707963267948966
    >>> normalize_radians(-np.pi/2)
    -1.5707963267948966
    """
    if data < pi and data >= -pi:
        return data
    ndata = data - int(data / (pi * 2)) * pi * 2
    if ndata >= pi:
        ndata -= pi * 2
    elif ndata < -pi:
        ndata += pi * 2
    return ndata


def prob(a, b):
    """ page 123 """
    return stats.norm.pdf(a, b)
    tmp = 1. / (sqrt(6) * b) - abs(a) / (6.0 * b)
    return max(0, tmp)


def mv_normal(mu, cov, pos):
    """
    >>> mu = array([0.])
    >>> cov = diag([1.])
    >>> pos = array([.0])
    >>> mv_normal(mu, cov, pos)
    0.3989422804014327

    TODO: this is not ideal
    """
    k = pos.shape[0]
    part1 = np.exp(-0.5 * k * np.log(2 * np.pi))
    #print 'part1', part1
    #print 'cov', cov
    #print 'det', np.linalg.det(cov)
    part2 = np.linalg.det(cov) ** -0.5
    #print 'part2', part2
    diff = pos - mu
    exp_ = np.exp(-0.5 * np.dot(np.dot(diff.T, np.linalg.pinv(cov)), diff))
    return part1 * part2 * exp_


###############################################################################
class Pose2D:
    def __init__(self, x=0, y=0, rot=0):
        self.x = x
        self.y = y
        self.rot = rot

    def __str__(self):
        return '/'.join([str(self.x), str(self.y), str(self.rot)])

    def rotate(self, rot):
        self.rot = normalize_radians(self.rot + rot)

    def translate(self, delta_x, delta_y):
        sin_ = sin(self.rot)
        cos_ = cos(self.rot)
        #self.x += delta_x
        self.x += (delta_x * cos_ - delta_y * sin_)
        #self.y += delta_y
        self.y += (delta_x * sin_ + delta_y * cos_)

    def rotate_and_translate(self, control):
        self.translate(control.x, control.y)
        self.rotate(control.rot)


def test_plot_cov_matrix():
    mean = zeros(2)
    sigma = ones((2, 2))
    import random
    sigma[0, 0] = random.random()
    sigma[0, 1] = random.random()
    sigma[1, 0] = random.random()
    sigma[1, 1] = random.random()
    def gauss():
        return np.random.multivariate_normal(mean, sigma, 1000)
    data = gauss()
    plot(data[:,0], data[:,1], '.')
    u, s, v = svd(sigma)
    #uncertainty_of_cov(mean, sigma)
    #draw()
    #show()


if __name__ == '__main__':
    #test_plot_cov_matrix()
    pass
