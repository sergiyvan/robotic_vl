#!/usr/bin/env python


"""
Kalman Filter as class

Kalman filter example demo in Python

A Python implementation of the example given in pages 11-15 of "An
Introduction to the Kalman Filter" by Greg Welch and Gary Bishop,
University of North Carolina at Chapel Hill, Department of Computer
Science, TR 95-041,
http://www.cs.unc.edu/~welch/kalman/kalmanIntro.html
"""

from __future__ import division

import numpy
import pylab


class KalmanFilter(object):
    """
    A simple stationary KF which saves all intermediate results.
    """
    def __init__(self, n, Q, R, x_guess, P_guess):
        super(KalmanFilter, self).__init__()

        # allocate space for arrays
        self.x = numpy.zeros(n)      # a posteri estimate of x
        self.x_bel = numpy.zeros(n)  # a priori estimate of x
        self.P = numpy.zeros(n)      # a posteri error estimate
        self.P_bel = numpy.zeros(n)  # a priori error estimate
        self.K = numpy.zeros(n)      # gain or blending factor

        self.Q = Q
        self.R = R

        self.x[0] = x_guess
        self.P[0] = P_guess

    def predict(self, u=None):
        self.x_bel[k] = self.x[k - 1]
        self.P_bel[k] = self.P[k - 1] + self.Q

    def update(self, z_new):
        self.K[k] = self.P_bel[k] / (self.P_bel[k] + self.R)
        self.x[k] = self.x_bel[k] + self.K[k] * (z_new - self.x_bel[k])
        self.P[k] = (1 - self.K[k]) * self.P_bel[k]


# numbers of iterations
n_iter = 50
# size of array
sz = (n_iter,)

# truth value
truth = -0.37727
# generate measured observations (normal about x, sigma=0.1)
z = numpy.random.normal(truth, 0.1, size=sz)
#z = numpy.sin(numpy.linspace(0, 2*numpy.pi, 50))


# kalman filter
kf = KalmanFilter(
        n=sz,         # dimension
        Q=1e-5,       # process variance (small)
        R=0.1**2,   # estimate of measurement variance
        x_guess=0.0,  # intial guesses
        P_guess=1.0   # intial guesses
    )

# iterate over measurements
for k in range(1, n_iter):
    kf.predict()
    kf.update(z_new=z[k])


# plot everything
# plot the measurements and estimates
pylab.figure()
pylab.subplot(311)
pylab.plot(z, 'k+', label='noisy measurements')
pylab.plot(kf.x, 'b-', label='state estimate: a posteri estimate')
pylab.axhline(truth, color='g', label='truth value')
pylab.legend()
pylab.xlabel('Iteration')
pylab.ylabel('Voltage')

# plot covariance: P_bel
pylab.subplot(312)
valid_iter = range(1, n_iter)  # P_bel not valid at step 0
pylab.plot(valid_iter, kf.P_bel[valid_iter], label='a priori error estimate')
pylab.xlabel('Iteration')
pylab.ylabel('$(Voltage)^2$')
pylab.setp(pylab.gca(), 'ylim', [0, .01])

# plot kalman gain
pylab.subplot(313)
pylab.plot(kf.K, label='Kalman Gain')
pylab.xlabel('Iteration')
pylab.ylabel('kalman gain')

# show everything
pylab.show()
