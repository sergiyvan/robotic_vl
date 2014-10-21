#!/usr/bin/env python

"""
UKF Localization from page 221
"""
from __future__ import division

import logging
logger = logging.getLogger()
print_handler = logging.StreamHandler()
print_handler.setLevel(logging.DEBUG)
logger.addHandler(print_handler)

from numpy import array
#from numpy import ones
from numpy import zeros
from numpy import degrees

import random

import numpy as np
from numpy import dot
from numpy import sqrt
from numpy.linalg import pinv

from numpy import sin
from numpy import cos
from numpy import arctan2

from scipy.linalg import svd

from helper_math import Pose2D
from helper_math import normalize_radians
from helper_math import mv_normal

from sqrtm import sqrtm

import pygame
from pygame import Rect
from pygame.locals import KEYDOWN
from pygame.locals import K_ESCAPE
from pygame.locals import K_RIGHT
from pygame.locals import K_q


from world import p2s
from world import World
from world import world2measurement


##############################################################################
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (200, 200, 200)
RED = (255, 0, 0)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)

##############################################################################
# globals :(
##############################################################################
global window
global pitch_surface


##############################################################################
# SIGMA POINTS
#
# init
# L is dimensionality of the state vector,
L = 3
# the number of sigma points
dim = 2 * L + 1  # 7


##############################################################################

##############################################################################
class Control:
    """
    Control: odometry motion model. See p. 133.
    """
    def __init__(self, x=0., y=0., rot=0.):
        self.x = x
        self.y = y
        self.rot = rot

        self.parameter1 = 0
        self.parameter2 = 0
        self.parameter3 = 0


##############################################################################
def update_pitch(surface, size, world):
    """ update the pitch """
    pygame.draw.rect(surface, GRAY, (0, 0, size[0], size[1]))
    pygame.draw.rect(surface, GREEN, (20, 40, 600, 400))

    # middle line
    pygame.draw.line(surface, WHITE, (320, 40), (320, 440), 5)

    # circle
    pygame.draw.circle(surface, WHITE, (320, 240), 60, 5)

    # field
    pygame.draw.rect(surface, WHITE,
            Rect(p2s((-300, 200)), (600, 400)), 5)

    # penalty area
    pygame.draw.rect(surface, WHITE,
            Rect(p2s((-300, 150)), (60, 300)), 5)
    pygame.draw.rect(surface, WHITE,
            Rect(p2s((240, 150)), (60, 300)), 5)

    # FEATURES
    pygame.draw.circle(surface, RED,
            p2s((world.BEACON.x, world.BEACON.y)), 5, 3)

    # circle
    pygame.draw.circle(surface, BLACK,
            p2s((world.circle.x, world.circle.y)), 5, 3)

    # goal goal
    pygame.draw.circle(surface, YELLOW,
            p2s((world.y1.x, world.y1.y)), 7)
    pygame.draw.circle(surface, YELLOW,
            p2s((world.y2.x, world.y2.y)), 7)

    # goal blue
    pygame.draw.circle(surface, BLUE,
            p2s((world.b1.x, world.b1.y)), 7)

    pygame.draw.circle(surface, BLUE,
            p2s((world.b2.x, world.b2.y)), 7)

    # yby
    pygame.draw.circle(surface, YELLOW,
            p2s((world.yby.x, world.yby.y)), 8)
    pygame.draw.circle(surface, BLUE,
            p2s((world.yby.x, world.yby.y)), 6)
    pygame.draw.circle(surface, YELLOW,
            p2s((world.yby.x, world.yby.y)), 3)

    # byb
    pygame.draw.circle(surface, BLUE,
            p2s((world.byb.x, world.byb.y)), 8)
    pygame.draw.circle(surface, YELLOW,
            p2s((world.byb.x, world.byb.y)), 6)
    pygame.draw.circle(surface, BLUE,
            p2s((world.byb.x, world.byb.y)), 3)

    for x in world.x_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y - 5)), p2s((x.x, x.y + 5)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x - 5, x.y)), p2s((x.x + 5, x.y)), 1)

    for x in world.t_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x - 5, x.y)), p2s((x.x + 5, x.y)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x, x.y - 10)), 1)

    for x in world.l_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x, x.y + 10)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x + 10, x.y)), 1)


##############################################################################
def _draw_line(left, right, color):
    p1 = (left[0], left[1])
    p2 = (right[0], right[1])
    pygame.draw.line(pitch_surface, color,
            p2s(p1), p2s(p2), 1)


def draw_cov(mean, sigma, zoom=100):
    """
    Fancy plot for covariance matrix

    """
    def _get_point_in_j_tm_main_axis(s, v, j, mean, zoom):
        """
        return the two points in the j-th main axis.

        s and v are part of the svd of the covariance
        """
        point = sqrt(s[j]) * v[j, :]
        point = point.reshape((2, 1))

        left = mean + (point * zoom)
        right = mean - (point * zoom)

        return left, right

    sigma = sigma[0:2, 0:2]
    u, s, v = svd(sigma)

    top_left, top_right = _get_point_in_j_tm_main_axis(s, v, 0, mean, zoom)
    btm_left, btm_right = _get_point_in_j_tm_main_axis(s, v, 1, mean, zoom)

    _draw_line(top_left, top_right, RED)
    _draw_line(btm_left, btm_right, RED)

    _draw_line(top_left, btm_left, RED)
    _draw_line(top_right, btm_right, RED)

    _draw_line(top_left, btm_right, RED)
    _draw_line(btm_left, top_right, RED)


def draw_pose(pose, color=BLACK, arrowlength=20):
    """
    draw a given pose on the field.

    results in a point for the position and a line indicating the angle.
    """
    pos_field = p2s(pose)
    pygame.draw.circle(pitch_surface, color, pos_field, 2)

    x = int(cos(pose[2, 0]) * arrowlength) + pose[0, 0]
    y = int(sin(pose[2, 0]) * arrowlength) + pose[1, 0]

    arrow_direction = p2s((x, y))
    pygame.draw.line(pitch_surface, color, pos_field, arrow_direction, 1)


def visualize_measurement(measurements, robot_pose):
    for key, measurement in measurements.items():
        distance = measurement[0, 0]
        angle = measurement[1, 0] + robot_pose[2, 0]

        x = int(cos(angle) * distance) + robot_pose[0, 0]
        y = int(sin(angle) * distance) + robot_pose[1, 0]

        pos = p2s((x, y))
        pygame.draw.circle(pitch_surface, BLACK, pos, 5, 1)


def visualize_kf(ukf, i):
    """
    visualize the given kf
    """
    print "TRACK %d: %f" % (i, ukf.p),
    print 'guessed state:',
    print int(ukf.mu[0]), int(ukf.mu[1]), degrees(ukf.mu[2])

    # draw cov matrix
    robot_pos = array([ukf.mu[0], ukf.mu[1]])
    draw_cov(robot_pos, ukf.Sigma, zoom=20)

    draw_pose(ukf.mu, YELLOW)


##############################################################################
class MultiHypothesesManager:
    """Manage several instances of KF instances."""
    def __init__(self):
        self.MAX_HYP = 5
        self.tracks = []

    def start(self):
        pass

    def execute(self, control, measurements):
        """
        do the job
        """
        # update all tracks
        for track in self.tracks:
            track.execute(control, measurements)

        #self._kill_tracks()

        # spawn new tracks
        pass

        # merge similar tracks
        self._merge_filters()

    def _are_filters_similar(self, track1, track2):
        diff_translation = np.linalg.norm(track1.mu[:2, 0] - track2.mu[:2, 0])
        diff_rotation = abs(track1.mu[2, 0] - track2.mu[2, 0])

        #print 'trans', diff_translation,
        #print 'rotat', diff_rotation

        TRANSLATION_THRESHOLD = 25.
        ROTATION_TRESHOLD = .5

        if (diff_translation < TRANSLATION_THRESHOLD and
                diff_rotation < ROTATION_TRESHOLD):
            return True
        return False

    def _merge_filters(self):
        """
        merge similar tracks
        """
        merged_idxs = []

        for i in range(len(self.tracks)):
            for j in range(i + 1, len(self.tracks)):

                #print '-' * 78
                #print 'comparing tracks %d/%d;' % (i, j)
                if self._are_filters_similar(self.tracks[i], self.tracks[j]):
                    print 'Tracks %d and %d are similar:' % (i, j), 'MERGING'
                    merged_idxs.append(j)
                    self.tracks[i].merge_with(self.tracks[j])
                else:
                    #print 'not similar'
                    pass

        # new tracks are all tracks that didn't get merged
        new_tracks = [self.tracks[i] for i in range(len(self.tracks))
                      if i not in merged_idxs]

        self.tracks = new_tracks

    def _kill_tracks(self):
        """
        remove tracks that are really unlikly
        """
        if len(self.tracks) > 3:
            # sort tracks in ascending order by probability
            self.tracks.sort(cmp=lambda x, y: cmp(x.p, y.p))

            # delete most unlikely tracks
            for track in self.tracks:
                if len(self.tracks) < 4:
                    break
                if track.p < 0.000001:
                    print 'removing track', track, 'with p', track.p
                    self.tracks.remove(track)

    def visualize(self, measurements):
        """
        Visualize every aspect of all tracks
        """
        print "number of tracks:", len(self.tracks)
        for i, track in enumerate(self.tracks):
            visualize_kf(track, i)
            visualize_measurement(measurements, track.mu)


##############################################################################
class UKF_unaugmented:
    def __init__(self, mu, Sigma, map_m):
        self.mu = mu
        self.Sigma = Sigma
        self.map_m = map_m

        self.mu_bel = None
        self.Sigma_bel = None

        self.w_m = None
        self.w_c = None
        self.lambda_ = None

        self._gen_sigma_weights()

    def _gen_sigma_weights(self):
        """
        Generate the weights for the sigma points.

        See "the unscented kalman filter" page 6
        """
        # alpha: spread of the sigma points arount the data.
        #        1 <= alpha <= 1e-4
        alpha = 1  # 1.
        # kappa: scaling parameter: determine spread from the mean
        #        Usally set to 0 or 3-L
        kappa = 0  # 3 - L
        # beta: encode additional (higher order) knowledge about the dist
        # 2 is optimal if dist is Gaussian
        beta = 2.
        # scaling parameter
        lambda_ = alpha ** 2 * (L + kappa) - L
        #print 'lambda:', lambda_

        # weights for computing the mean
        w_m = zeros((dim, 1))
        w_m[0] = lambda_ / (L + lambda_)
        #print lambda_ / (L + lambda_)
        #print w_m[0]
        for i in range(1, 2 * L + 1):
            w_m[i] = 1 / (2 * (L + lambda_))

        # weights for computing the cov
        w_c = w_m.copy()
        w_c[0] = lambda_ / (L + lambda_) + (1 - alpha ** 2 + beta)

        #logger.debug('w_m:' + repr(w_m.T))
        #logger.debug('w_c:' + repr(w_c.T))

        #print 'w_m:' + repr(w_m.T)
        #print '\t', sum(w_m)
        #print 'w_c:' + repr(w_c.T)
        #print '\t', sum(w_c)

        self.w_m = w_m
        self.w_c = w_c
        self.lambda_ = lambda_

    def _gen_sigma_points(self, mu, Sigma):
        """
        Generate Sigma points.
        The weights are constant.

        For all details see eq. 3.66 on page 65ff
        """
        # 6
        # scaling parameter: determine spread from the mean
        # 1 < alpha < 1e-4
        # see "the unscented kalman filter" page 6
        # see eq. 3.66
        # Sigma_augmented
        sigma_points = zeros((L, dim))  # 3 zeilen, 7 spalten
        mu = mu.reshape((3,))
        sigma_points[:, 0] = mu

        # mu_a + gamma * sqrt(Sigma_a)
        sqrt_sigma_a = sqrtm((L + self.lambda_) * Sigma)
        print 'sqrt-sigma', sqrt_sigma_a

        for i in range(1, L + 1):
            sigma_points[:, i] = mu + sqrt_sigma_a[:, i - 1]

        # mu_a - gamma * sqrt(Sigma_a))
        for i in range(L + 1, 2 * L + 1):
            sigma_points[:, i] = mu - sqrt_sigma_a[:, i - L - 1]

        assert (3, 7) == sigma_points.shape

        print 'sigma points:', sigma_points

        logger.debug('sigma_points[:, 2]:' + repr(sigma_points[:, 2].T))
        logger.debug('sigma_points[:, 3]:' + repr(sigma_points[:, 3].T))
        return sigma_points

    def _measurement_model(self, sigma_points, measurement, msmt_name):
        """
        feature based measurement model.
        """
        Z_bel = zeros((2, dim))
        beacon = getattr(self.map_m, msmt_name)
        print 'name', msmt_name

        print 'real measu:', measurement.reshape((2,))
        for i in range(dim):
            sp = sigma_points[:, i]
            Z_bel[:, i] = array([
                    sqrt((beacon.x - sp[0]) ** 2 + (beacon.y - sp[1]) ** 2),
                    normalize_radians(
                        normalize_radians(
                            arctan2(beacon.y - sp[1], beacon.x - sp[0]))
                        - sp[2])
                    ])
            print 'sigma meas:', i, sp
            print 'after motion:', Z_bel[0, i], degrees(Z_bel[1, i]), 'in deg'
        assert (2, 7) == Z_bel.shape

        return Z_bel

    def _motion_model(self, X, ctrl_u):
        """
        Odometry motion model
        """
        X_bel = zeros((3, dim))
        for i in range(dim):
            # select pose
            pose = Pose2D(x=X[0, i],
                          y=X[1, i],
                          rot=X[2, i])
            # move robot
            pose.rotate_and_translate(ctrl_u)

            # set the result
            X_bel[0:, i] = pose.x
            X_bel[1:, i] = pose.y
            X_bel[2:, i] = pose.rot

            # draw sigma points
            pygame.draw.circle(pitch_surface, BLACK,
                    p2s((pose.x, pose.y)), 9, 1)
            print 'X_bel:', X_bel[0, i], X_bel[1, i], X_bel[2, i],
            moved = sqrt((X[0, i] - pose.x) ** 2 + (X[1, i] - pose.y) ** 2)
            print ' moved:', moved

        assert (3, 7) == X_bel.shape
        return X_bel

    def execute(self, ctrl_u, measurements):
        """
        do a prediction and correction step

        ctrl_u: Control/odometrie
        measurement_z: features {z1, z2, z3, ...}
        """

        # MOVE
        # motion noise
        R = np.diag([.1 + 0.1 * abs(ctrl_u.x),
                     .1 + 0.1 * abs(ctrl_u.y),
                     .1 + 0.02 * abs(ctrl_u.rot)])
        self.move(ctrl_u, R)

        # SENSE
        self.p = 1
        for key, measurement in measurements.items():
            print '=' * 70
            print "\tSENSE:", key, "at", measurement.T
            print '=' * 70
            # measurement noise
            Q = np.diag([.2 + 0.1 * abs(measurement[0, 0]),
                         .1 + 0.2 * abs(measurement[1, 0])])
            self.sense(key, measurement, Q)

        self.mu = self.mu_bel
        self.Sigma = self.Sigma_bel

    def move(self, ctrl_u, R):
        """
        Generate Sigma points
        """
        print ' 2 ' + '=' * 70
        sigma_points = self._gen_sigma_points(self.mu, self.Sigma)
        for i in range(dim):
            print 'sigma:', sigma_points[:, i]

        #######################################################################
        # pass sigma points through the motion model and compute gaussian stats
        #######################################################################
        # 3
        # estimate sigma points by applying the motion model
        #X_bel = g(ctrl_u, X_a_state_x)
        print ' 3 ' + '=' * 70
        X_bel = self._motion_model(sigma_points, ctrl_u)

        # 4
        #mu_bel = sum(w_m X_x_bel)
        print ' 4 ' + '=' * 70
        tmp = 0.
        for i in range(dim):
            print 'X_bel   ', X_bel[:, i]
            print 'weight  ', self.w_m[i]
            print 'weighted', X_bel[:, i] * self.w_m[i]
            tmp += X_bel[:, i] * self.w_m[i]
            print '=---------------'
        print 'result', tmp

        moved = np.linalg.norm(self.mu - tmp)
        print 'tmp', tmp
        print 'mu', self.mu.shape
        print '', self.mu - tmp
        print ' moved:', moved

        self.mu_bel = dot(X_bel, self.w_m)
        #w = ones((7, 1)) * 0.14285714285714285
        #self.mu_bel = dot(X_bel, w)
        logger.debug('mu_bel:' + repr(self.mu_bel))
        print 'mu_bel: ' + repr(self.mu_bel.T),
        moved = sqrt((self.mu[0, 0] - self.mu_bel[0, 0]) ** 2 +
                ((self.mu[1, 0] - self.mu_bel[1, 0]) ** 2))
        print ' moved:', moved
        assert (3, 1) == self.mu_bel.shape

        # 5
        # Sigma_bel = sum(w_c * (X_x_bel - mu_bel) * (X_x_bel - mu_bel).T)
        print ' 5 ' + '=' * 70
        X_minus_mu = X_bel - self.mu_bel
        self.Sigma_bel = dot(self.w_c.T * X_minus_mu, (X_minus_mu).T) + R
        assert (3, 3) == self.Sigma_bel.shape

    def sense(self, msmt_name, measurement, Q):
        """
        predict observations at sigma points and compute Gaussian statistics
        """
        print ' 6 ' + '=' * 70
        # 6
        print "\tgen sigma points"
        sigma_points = self._gen_sigma_points(self.mu_bel, self.Sigma_bel)

        print sigma_points.shape
        for i in range(dim):
            print 'msmt sigma:', sigma_points[:, i]

        # 7
        print ' 7 ' + '=' * 70
        print "\tapply measurement model to sigma points and measurement"
        Z_bel = self._measurement_model(sigma_points, measurement, msmt_name)
        #print 'Z_bel', Z_bel

        # 8
        print ' 8 ' + '=' * 70
        print "\tmean of predicted measurement", msmt_name
        z_bel = dot(Z_bel, self.w_m)
        print 'z_bel:', z_bel.T, 'in deg', degrees(z_bel[1, 0])
        assert (2, 1) == z_bel.shape

        # 9
        print ' 9 ' + '=' * 70
        print "\tcovariance of predicted measurement", msmt_name
        print 'Q', Q
        S = dot(self.w_c.T * (Z_bel - z_bel), (Z_bel - z_bel).T) + Q
        print 'S', S
        assert (2, 2) == S.shape

        # 10
        print '10 ' + '=' * 70
        print "\tcross covariance between robot location and ovservation"
        crosscov_x_z = dot(self.w_c.T * (sigma_points - self.mu_bel),
                           (Z_bel - z_bel).T)
        print crosscov_x_z
        assert (3, 2) == crosscov_x_z.shape

        #######################################################################
        # update mean and cov
        #######################################################################
        # 11
        print '11 ' + '=' * 70
        print "\tcalculate the Kalman gain"
        K = dot(crosscov_x_z, pinv(S))
        print K

        # 12
        print '12 ' + '=' * 70
        print "\tmu measurement update"
        self.mu_bel = self.mu_bel + dot(K, (measurement - z_bel))
        self.mu_bel[2, 0] = normalize_radians(self.mu_bel[2, 0])
        print 'mu:', self.mu_bel.T, 'mu deg:', np.degrees(self.mu_bel[2, 0])
        assert (3, 1) == self.mu_bel.shape

        # 13
        # Sigma measurement update
        print '13 ' + '=' * 70
        print "\tSigma measurement update"
        self.Sigma_bel = self.Sigma_bel - dot(dot(K, S), K.T)
        assert (3, 3) == Sigma.shape

        # 14
        print '14 ' + '=' * 70
        print '\tMultivar. gauss'
        p = mv_normal(measurement, S, z_bel)
        print 'p:', p

        # 15
        print '15 ' + '=' * 70
        self.mu = self.mu_bel
        self.Sigma = self.Sigma_bel
        self.p *= p

    def merge_with(self, other):
        """
        Merge this UKF with the other ukf.

        Simplified version of:
        'Multiple Model Kalman Filters: A Localization Technique for RCS'

        TODO: replace .5 by weight of GD.

        If two GD are the same:
        >>> ukf1 = UKF_unaugmented(np.ones(1), 2 * np.ones(1), None)
        >>> ukf2 = UKF_unaugmented(np.ones(1), 2 * np.ones(1), None)
        >>> ukf1.merge_with(ukf2)
        >>> print ukf1.mu
        [ 1.]
        >>> print ukf1.Sigma
        [ 2.]

        Same in 2D:
        >>> mean = np.zeros(2)
        >>> cov = 2 * np.ones((2,2))
        >>> ukf1 = UKF_unaugmented(mean, cov, None)
        >>> ukf2 = UKF_unaugmented(mean, cov, None)
        >>> ukf1.merge_with(ukf2)
        >>> print ukf1.mu
        [ 0.  0.]
        >>> print ukf1.Sigma
        [[ 2.  2.]
         [ 2.  2.]]

        Same mean, different cov:
        #>>> mean = np.ones(2)
        #>>> cov = np.ones((2,2))
        #>>> ukf1 = UKF_unaugmented(mean, cov, None)
        #>>> ukf2 = UKF_unaugmented(mean, cov * 2, None)
        #>>> ukf1.merge_with(ukf2)
        #>>> print ukf1.mu
        #[ 1.  1.]
        #>>> print ukf1.Sigma
        #[[ 2.  2.]
         #[ 2.  2.]]

        FIXME does it make sense, that the cov grows?
        #>>> mean = np.ones(2)
        #>>> cov = np.ones((2, 2))
        #>>> ukf1 = UKF_unaugmented(mean, cov, None)
        #>>> ukf2 = UKF_unaugmented(mean * 3, cov, None)
        #>>> ukf1.merge_with(ukf2)
        #>>> print ukf1.mu
        #[ 2.  2.]
        #>>> print ukf1.Sigma
        #[[ 2.  2.]
         #[ 2.  2.]]
        """
        new_mean = .5 * self.mu + .5 * other.mu

        diff_self = self.mu - new_mean
        diff_other = other.mu - new_mean

        new_cov = (.5 / 1 * (self.Sigma + dot(diff_self, diff_self.T)) +
                   .5 / 1 * (other.Sigma + dot(diff_other, diff_other.T)))

        self.mu = new_mean
        self.Sigma = new_cov
        # TODO update p/trust/weight


##############################################################################
if __name__ == '__main__':

    #########################################
    # init UKF
    #########################################

    Sigma = np.diag([.9, .9, .9]) * 100
    ctrl_u = Control(10., 0., 0.0)

    # measurements
    msmt_beacon = array([[400.], [0.]])
    msmt_byb = world2measurement(Pose2D(-100., 0., 0.),
                                 World.byb)
    measurements = {'BEACON': msmt_beacon,
                    'byb': msmt_byb}

    real_pos = array([[-100., 0., 0]]).T
    test_pos1 = array([[-200., 50., 0]]).T
    test_pos2 = array([[-200., 50., -2.0]]).T
    test_pos3 = array([[00., 50., .6]]).T
    assert (3, 1) == real_pos.shape

    mh_manager = MultiHypothesesManager()
    mh_manager.tracks.append(UKF_unaugmented(real_pos, Sigma, World))
    mh_manager.tracks.append(UKF_unaugmented(test_pos1, Sigma, World))
    mh_manager.tracks.append(UKF_unaugmented(test_pos2, Sigma, World))
    mh_manager.tracks.append(UKF_unaugmented(test_pos3, Sigma, World))

    #########################################
    # PYGAME
    #########################################
    # init pygame stuff to display everything
    window_size = (640, 480)
    pygame.init()
    window = pygame.display.set_mode(window_size)
    pygame.key.set_repeat(200, 5)

    pygame.display.flip()

    # pitch / soccer field
    pitch_pos = (0, 0)
    pitch_size = (640, 480)
    pitch_surface = pygame.Surface(pitch_size)
    update_pitch(pitch_surface, pitch_size, World)
    window.blit(pitch_surface, pitch_pos)
    pygame.display.update()

    done = False
    data = None
    # input handling and main loop
    while not done:
        # process all events
        for event in pygame.event.get():
            # QUIT
            if event.type == pygame.QUIT:
                print "Aborting...wait a sec..."
                done = True
                break

            elif event.type == KEYDOWN:
                # QUIT
                if event.key == K_ESCAPE or event.key == K_q:
                    print "Aborting...wait a sec..."
                    done = True
                    break

                # process next frame
                elif event.key == K_RIGHT:

                    update_pitch(pitch_surface, pitch_size, World)

                    # set parameter for one step
                    ctrl_u.x = 10 * random.random() - 2
                    ctrl_u.y = 10 * random.random() - 2
                    #ctrl_u.rot = random.random()
                    #ctrl_u.y = 5 * random.random()

                    # update real_pos according to control
                    real_pos[0, 0] += ctrl_u.x
                    real_pos[1, 0] += ctrl_u.y
                    real_pos[2, 0] += ctrl_u.rot

                    # update measurements accordingly
                    real_pose2d = Pose2D(real_pos[0, 0],
                                         real_pos[1, 0],
                                         real_pos[2, 0])

                    # update beacon (add noise)
                    beacon = world2measurement(real_pose2d, World.BEACON)
                    beacon[0, 0] = beacon[0, 0] + (beacon[0, 0] *
                                                   (np.random.randn() / 10.))
                    beacon[1, 0] = beacon[1, 0] + np.random.randn() / 10

                    # update byb (add noise)
                    byb = world2measurement(real_pose2d, World.byb)
                    byb[0, 0] += np.random.randn() * 10
                    byb[1, 0] = byb[1, 0] + np.random.randn() / 10

                    measurements['byb'] = byb
                    measurements['BEACON'] = beacon
                    # print ground truth
                    print 'real pos (black)', real_pos.reshape((3,))
                    draw_pose(real_pos)

                    print 'ctrl:', ctrl_u.x, ctrl_u.y, ctrl_u.rot
                    print 'measurement', msmt_beacon.reshape((2,))

                    # execute kf
                    mh_manager.execute(ctrl_u, measurements)

                    # visualize stuff
                    mh_manager.visualize(measurements)

                    print '=' * 70

                    # update screen
                    window.blit(pitch_surface, pitch_pos)
                    pygame.display.update()
