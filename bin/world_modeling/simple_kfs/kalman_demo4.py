#!/usr/bin/env python


"""
Non stationary version

Kalman filter example demo in Python

A Python implementation of the example given in pages 11-15 of "An
Introduction to the Kalman Filter" by Greg Welch and Gary Bishop,
University of North Carolina at Chapel Hill, Department of Computer
Science, TR 95-041,
http://www.cs.unc.edu/~welch/kalman/kalmanIntro.html
"""

from __future__ import division

import numpy as np
import pylab


class KalmanFilter(object):
    """
    A simple stationary KF which saves all intermediate results.
    """
    def __init__(self, n, x_guess, P_guess, Q, R, A, H, B):
        """
        x state (n)
        z mesurement (m)
        u control (l)

        A (n*n): x_k = A*x_k-1
        B (n*l): dx = B*u
        H (m*n): z_k = H*x_k
        Q (n*n) process noise covariance m.rand(3,4)
        R (m*m) measurement noise covariance
        P (n*n) state noise covariance
        K Kalman gain
        """
        super(KalmanFilter, self).__init__()

        # allocate space for arrays
        self.x = np.zeros(n)      # a posteri estimate of x
        self.x_bel = np.zeros(n)  # a priori estimate of x
        self.P = np.zeros(n)      # a posteri error estimate
        self.P_bel = np.zeros(n)  # a priori error estimate
        self.K = np.zeros(n)      # gain or blending factor

        self.Q = Q
        self.R = R
        self.A = A
        self.H = H
        self.B = B
        self.I = pylab.eye(1)

        self.x[0] = x_guess[0]
        self.P[0] = P_guess

    def predict(self, u):
        self.x_bel[k] = self.A * self.x[k-1] + self.B * u
        self.P_bel[k] = self.A * self.P[k-1] * self.A.T + self.Q

    def update(self, z_new):
        self.K[k] = self.P_bel[k] * self.H.T / (
                self.H * self.P_bel[k] * self.H.T + self.R)
        self.x[k] = self.x_bel[k] + self.K[k] * (z_new - self.H * self.x_bel[k])
        self.P[k] = (self.I - self.K[k] * self.H) * self.P_bel[k]


# numbers of iterations
n_iter = 100
# size of array
sz = (n_iter,)

# truth value
truth = np.sin(np.linspace(0, 6 * np.pi, n_iter)) - 0.37727
# generate measured observations (normal about x, sigma=0.1)
z = np.random.normal(truth, .1)

x_guess=np.ndarray([0.0]),  # intial guesses

# kalman filter
kf = KalmanFilter(
        n=sz,         # dimension
        x_guess=np.array([0.0]),  # intial guesses
        P_guess=np.array([1.0]),  # intial guesses
        #Q=1e-5,       # process variance (small)
        Q=np.array([0.0001]),       # process variance (small)
        R=np.array([0.1 ** 2]),      # estimate of measurement variance
        A=np.array([1.]),
        H=np.array([1.]),
        B=np.array([1.])
    )

# iterate over measurements
for k in range(1, n_iter):
    kf.predict(np.array([0]))
    kf.update(z_new=z[k])


# plot everything
# plot the measurements and estimates
pylab.figure()
pylab.subplot(311)
pylab.grid()
pylab.plot(z, 'k+', label='noisy measurements')
pylab.plot(kf.x, 'b-', label='state estimate: a posteri estimate')
pylab.plot(truth, color='g', label='truth value')
pylab.legend()
pylab.xlabel('Iteration')
pylab.ylabel('Voltage')

# plot covariance: P_bel
pylab.subplot(312)
pylab.grid()
valid_iter = range(1, n_iter)  # P_bel not valid at step 0
pylab.plot(valid_iter, kf.P_bel[valid_iter], label='a priori error estimate')
pylab.xlabel('Iteration')
pylab.ylabel('$(Voltage)^2$')
pylab.setp(pylab.gca(), 'ylim', [0, .01])

# plot kalman gain
pylab.subplot(313)
pylab.grid()
pylab.plot(kf.K, label='Kalman Gain')
pylab.xlabel('Iteration')
pylab.ylabel('kalman gain')

# show everything
pylab.show()
