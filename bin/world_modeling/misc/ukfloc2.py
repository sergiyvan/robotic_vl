#!/usr/bin/env python

"""
UKF Localization from page 221
"""
from __future__ import division

import sys
import pprint
import logging
logger = logging.getLogger()
print_handler = logging.StreamHandler()
print_handler.setLevel(logging.ERROR)
logger.addHandler(print_handler)

from numpy import array
from numpy import ones
from numpy import zeros
from numpy import degrees

import random

from numpy import dot
from numpy import sqrt
from numpy import pi
from numpy.linalg import pinv

from numpy import sin
from numpy import cos
from numpy import arctan2

from scipy.linalg import svd

from helper_math import Pose2D

from sqrtm import sqrtm
from sqrtm import _assert_sqrt

import pygame
from pygame import Rect
from pygame.locals import KEYDOWN
from pygame.locals import K_ESCAPE
from pygame.locals import K_RETURN
from pygame.locals import K_RIGHT
from pygame.locals import K_LEFT
from pygame.locals import K_q


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
def info(o):
    print type(o)
    print o.shape


def p2s(pos):
    """
    pitch to surface.

    translate pitch coordinates to surface coordinates
    """
    return 320 + int(pos[0]), 240 - int(pos[1])

##############################################################################

class World:
    """
                                     yby

      L-------------------------------T-------------------------------L
      |                               |                               |
      |                               |                               |
      |                               |                               |
      T-----L                         |                         L-----T
      |     |                         |                         |     |
     b|     |                     /---x---\                     |     |y
      |     |                    /    |    \                    |     |
      |     |                   |     |     |                   |     |
      |     |                   |     C     |                   |     | BEACON
      |     |                   |     |     |                   |     |
      |     |                    \    |    /                    |     |
     b|     |                     \---x---/                     |     |y
      |     |                         |                         |     |
      T-----L                         |                         L-----T
      |                               |                               |
      |                               |                               |
      |                               |                               |
      L-------------------------------T-------------------------------L

                                   byb

    For more infos see the robocup rules:
    - http://www.tzi.de/humanoid/bin/view/Website/WebHome
    """
    class point:
        def __init__(self, x, y):
            self.x = x
            self.y = y

    BEACON = point(300, 0)

    # goal yellow
    y1 = point(300, 75)
    y2 = point(300, -75)

    # goal blue
    b1 = point(-300, 75)
    b2 = point(-300, -75)

    yby = point(0, 240)
    byb = point(0, -240)

    x_features = [point(0, 60), point(0, -60)]

    t_features = [point(*p) for p in [
                    (0, -200), (0, 200),
                    (-300, -150), (-300, 150),
                    (300, -150), (300, 150)
            ]]

    l_features = [point(*p) for p in [
                    (-300, -200), (-300, 200),  # field corners
                    (300, -200), (300, 200),  # field corners
                    (-240, -150), (-240, 150),  # goal corners
                    (240, -150), (240, 150)  # goal corners
            ]]

    circle = point(0, 0)


##############################################################################
# SIGMA POINTS
#
# init
# L is dimensionality of the augmented state vector, given by the sum
# of state (3), control (3) and measurement (2) dimensions.
L = 8
# the number of sigma points
dim = 2 * L + 1  # 17


def _gen_sigma_points(mu_a, Sigma_a):
    """
    Generate Sigma points.

    The new augmented parts mu_a and Sigma_a are used to create the
    Sigma-Points

    The weights are constant.

    TODO: extract the calculation of the weights

    For all details see eq. 3.66 on page 65ff
    """
    # 6
    # dim(X_augmented) = 2 * L + 1 = 17
    # scaling parameter: determine spread from the mean
    # 1 < alpha < 1e-4
    # see "the unscented kalman filter" page 6
    alpha = .1
    # scaling parameter: determine spread from the mean
    # Usally set to 0 or 3-L
    # see "the unscented kalman filter" page 6
    kappa = 0.0
    # beta: encode additional (higher order) knowledge about the dist
    # 2 is optimal if dist is Gaussian
    beta = 2.
    # scaling parameter
    lambda_ = alpha * alpha * (L + kappa) - L

    # weights for computing the mean
    w_m = zeros((dim, 1))
    w_m[0] = lambda_ / (L + lambda_)
    for i in range(1, 2 * L + 1):
        w_m[i] = 1 / (2 * (L + lambda_))

    # weights for computing the cov
    w_c = w_m.copy()
    w_c[0] = lambda_ / (L + lambda_) + (1 - alpha * alpha + beta)

    logger.debug('w_m:' + repr(w_m.T))
    logger.debug('w_c:' + repr(w_c.T))

    # see eq. 3.66
    # Sigma_augmented
    X_augmented = zeros((L, dim))  # 8 zeilen, 17 spalten
    X_augmented[:, 0] = mu_a

    for i in range(L):
        assert X_augmented[i, 0] == mu_a[i]

    # mu_a + gamma * sqrt(Sigma_a)
    tmp = (L + lambda_) * Sigma_a
    sqrt_sigma_a = sqrtm(tmp)

    #_assert_sqrt(tmp, sqrt_sigma_a)

    for i in range(1, L + 1):
        X_augmented[:, i] = mu_a + sqrt_sigma_a[:, i - 1]

    # mu_a - gamma * sqrt(Sigma_a))
    for i in range(L + 1, 2 * L + 1):
        X_augmented[:, i] = mu_a - sqrt_sigma_a[:, i - L - 1]

    assert (8, 17) == X_augmented.shape

    logger.debug('X_augmented[:, 2]:' + repr(X_augmented[:, 2].T))
    logger.debug('X_augmented[:, 3]:' + repr(X_augmented[:, 3].T))
    return X_augmented, w_c, w_m


##############################################################################
def _motion_model_odometry(X_a_ctrl_u, X_a_state_x, ctrl_u):
    X_x_bel = zeros((3, dim))
    for i in range(dim):
        # select pose
        pose_augmented = Pose2D(x=X_a_state_x[0, i],
                                y=X_a_state_x[1, i],
                                rot=X_a_state_x[2, i])
        # create augmented control
        ctrl_augmented = _control_odometry_u(
                                        x=ctrl_u.x + X_a_ctrl_u[0, i],
                                        y=ctrl_u.y + X_a_ctrl_u[1, i],
                                        rot=ctrl_u.rot + X_a_ctrl_u[2, i])
        # move robot
        pose_augmented.rotate_and_translate(ctrl_augmented)

        # set the result
        X_x_bel[0:, i] = pose_augmented.x
        X_x_bel[1:, i] = pose_augmented.y
        X_x_bel[2:, i] = pose_augmented.rot

        # draw augmented sigma points
        pygame.draw.circle(pitch_surface, BLACK,
                p2s((pose_augmented.x, pose_augmented.y)), 9, 1)

    assert (3, 17) == X_x_bel.shape
    return X_x_bel


def _motion_model_velocity(X_a_ctrl_u, X_a_state_x, ctrl_u):
    """
    function of the motion model (also called f)
    """
    X_x_bel = zeros((3, 15))
    for i in range(dim):
        delta_t = 1
        # FIXME we don't want it to be zero otherwise we get division by zero
        # so we add some small value. find a better solution.
        vi = ctrl_u.v + X_a_ctrl_u[0, i] + 0.00000001
        wi = ctrl_u.w + X_a_ctrl_u[1, i] + 0.00000001
        thetai = X_a_state_x[2, i] + 0.00000001

        tmp = array([
            [-(vi / wi) * sin(thetai) + (vi - wi) * sin(thetai + wi * delta_t)],
            [(vi / wi) * cos(thetai) - (vi / wi) * cos(thetai + wi * delta_t)],
            [wi * delta_t]]).T

        X_x_bel[:, i] = X_a_state_x[:, i] + tmp
    assert (3, 15) == X_x_bel.shape

    return X_x_bel

motion_model = _motion_model_odometry


##############################################################################
class _control_velocity_u:
    """
    control: velocity motion model

    See chapter 5.3
    """
    def __init__(self, v=None, w=None):
        self.v = v if v else 0.000001
        self.w = w if w else 0.000001

        # and also some noise parameter
        self.a1 = .1
        self.a2 = .1
        self.a3 = .1
        self.a4 = .1


class _control_odometry_u:
    """
    control: odometry motion model. See p. 133.
    """
    def __init__(self, x=0., y=0., rot=0.):
        self.x = x
        self.y = y
        self.rot = rot

        self.parameter1 = 0
        self.parameter2 = 0
        self.parameter3 = 0


control = _control_odometry_u


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
            p2s((world.y1.x, world.y1.y)), 5, 3)
    pygame.draw.circle(surface, YELLOW,
            p2s((world.y2.x, world.y2.y)), 5, 3)

    # goal blue
    pygame.draw.circle(surface, BLUE,
            p2s((world.b1.x, world.b1.y)), 5, 3)

    pygame.draw.circle(surface, BLUE,
            p2s((world.b2.x, world.b2.y)), 5, 3)

    for x in world.x_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y-5)), p2s((x.x, x.y+5)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x-5, x.y)), p2s((x.x+5, x.y)), 1)

    for x in world.t_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x-5, x.y)), p2s((x.x+5, x.y)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x, x.y-10)), 1)

    for x in world.l_features:
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x, x.y+10)), 1)
        pygame.draw.line(surface, BLACK,
                p2s((x.x, x.y)), p2s((x.x+10, x.y)), 1)

    #pygame.draw.circle(surface, BLUE,
            #p2s((world..x, world..y)), 5, 2)
    #pygame.draw.circle(surface, RED,
            #p2s((world..x, world..y)), 5, 2)
    #pygame.draw.circle(surface, RED,
            #p2s((world..x, world..y)), 5, 2)

##############################################################################
def _draw_line(left, right, color):
    p1 = (left[0], left[1])
    p2 = (right[0], right[1])
    pygame.draw.line(pitch_surface, color,
            p2s(p1), p2s(p2), 1)


def draw_cov(mean, sigma, zoomfactor=100):
    """
    Fancy plot for covariance matrix

    """
    def _get_point_in_j_tm_main_axis(s, v, j, mean, zoomfactor):
        """
        return the two points in the j-th main axis.

        s and v are part of the svd of the covariance
        """
        point = sqrt(s[j]) * v[j, :]
        point = point.reshape((2, 1))

        left = mean + (point * zoomfactor)
        right = mean - (point * zoomfactor)

        return left, right

    sigma = sigma[0:2, 0:2]
    u, s, v = svd(sigma);

    top_left, top_right = _get_point_in_j_tm_main_axis(s, v, 0, mean, zoomfactor)
    btm_left, btm_right = _get_point_in_j_tm_main_axis(s, v, 1, mean, zoomfactor)

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

    x = int(cos(pose[2, 0]) * arrowlength) + pose[0,0]
    y = int(sin(pose[2, 0]) * arrowlength) + pose[1,0]

    arrow_direction = p2s((x, y))
    pygame.draw.line(pitch_surface, color, pos_field, arrow_direction, 1)


def visualize_measurement(measurements, robot_pose):
    #for measurement in measurements:
    measurement = measurements
    distance = measurement[0, 0]
    angle = measurement[1, 0] + robot_pose[2, 0]

    x = int(cos(angle) * distance) + robot_pose[0,0]
    y = int(sin(angle) * distance) + robot_pose[1,0]

    pos = p2s((x, y))
    pygame.draw.circle(pitch_surface, BLACK, pos, 5, 1)

    #print 'pos of measurement relativ', x, y
    #print 'pos of measurement', pos


def visualize_kf(ukf):
    """
    visualize the given kf
    """
    print 'guessed state:'
    print int(ukf.mu_old[0]), int(ukf.mu_old[1]), degrees(ukf.mu_old[2])

    # draw cov matrix
    robot_pos = array([ukf.mu_old[0], ukf.mu_old[1]])
    draw_cov(robot_pos, ukf.Sigma_old, zoomfactor=2)

    draw_pose(ukf.mu_old, YELLOW)

##############################################################################
class UKF:
    def __init__(self, mu, Sigma, map_m):
        self.mu_old = mu
        self.Sigma_old = Sigma
        self.map_m = map_m

        # create the augmented cov Sigma_a
        self.Sigma_a = zeros((L, L))

    def _measurement_model(self, X_x_bel, X_a_msmt_z):
        """
        feature based measurement model.
        """
        Z_bel = zeros((2, dim))
        beacon = self.map_m.BEACON
        for i in range(dim):
            tmp = array([
                sqrt((beacon.x - X_x_bel[0, i])**2 + (beacon.y - X_x_bel[1, i])**2),
                arctan2(beacon.y - X_x_bel[1, i], beacon.x - X_x_bel[1, i]) + X_x_bel[2, i]
                ])
            Z_bel[:, i] = tmp + X_a_msmt_z[:, i]
        assert (2, 17) == Z_bel.shape

        return Z_bel

    def execute(self, ctrl_u, measurement_z):
        """
        do a prediction and correction step

        ctrl_u: control/odometrie
        measurement_z: features {z1, z2, z3, ...}

        only one feature observation with exact correspondence
        """
        ##########################################################################
        #  generate augmented mean and covariance
        ##########################################################################
        # To get an more accurate representation of the influence of the noise
        # of the control and measurement, we augment the state with noise of
        # the control and measurement.
        #
        # We first construct control noise covariance matrix M_cov and the
        # measurement noise covariance matrix Q_cov. They are used to create the
        # augmented covariance of the state/mean and augmented state covariance.

        # 2
        # M_cov: motion model covariance matrix
        # M_cov (3x3)
        #M_cov_for_velocity_motion_model = array
            #[ctrl_u.a1 * (ctrl_u.v * ctrl_u.v) + ctrl_u.a2 * (ctrl_u.w * ctrl_u.w), 0],
            #[0, ctrl_u.a3 * (ctrl_u.v * ctrl_u.v) + ctrl_u.a4 * (ctrl_u.w * ctrl_u.w)]
            #])
        # TODO how to set the motion cov best?
        M_cov = array([
            [1.0, 0.1, 0.1],
            [0.1, 1.0, 0.1],
            [0.1, 0.1, 1.0]
            ])
        assert (3, 3) == M_cov.shape

        # 3
        # create measurement noise cov Q_cov
        # Q_cov (2x2)
        # range_noise: noise for the range part of the measurement
        range_noise = 2.2
        # bearing_noise: noise for the bearing part of the measurement
        bearing_noise = 0.3

        Q_cov = array([[range_noise,    0.              ],
                       [0.,             bearing_noise   ]])

        assert (2, 2) == Q_cov.shape

        # 4
        # mean_augmented = mean of loc, zero for control and measurement noise
        # dim of mu_a == 8 == L
        #
        # the control and measurement noise does not have an impact here.
        mu_a = zeros(L)
        mu_a[0:3] = self.mu_old.T
        assert (L, ) == mu_a.shape
        logger.debug('mu_a:' + repr(mu_a))
        #print 'mu_a:' + str((
                #int(mu_a[0]), int(mu_a[1]), int(mu_a[2]),
                #int(mu_a[3]), int(mu_a[4]), int(mu_a[5]),
                #int(mu_a[6]), int(mu_a[7])))

        # 5
        # create the augmented cov Sigma_a
        # Sigma_a (7x7)
        #
        #                 |sigma       0       0 |
        # augmented cov = |    0   M_cov       0 |
        #                 |    0       0   Q_cov |
        self.Sigma_a[0:3, 0:3] = self.Sigma_old.copy()
        self.Sigma_a[3:6, 3:6] = M_cov
        self.Sigma_a[6:8, 6:8] = Q_cov
        assert (L, L) == self.Sigma_a.shape
        logger.debug('Sigma_a:' + repr(self.Sigma_a))

        ##########################################################################
        # Generate Sigma points
        ##########################################################################
        # 6
        X_augmented, w_c, w_m = _gen_sigma_points(mu_a, self.Sigma_a)
        # divide Sigma points X_augmented in augmented subparts
        X_a_state_x = X_augmented[0:3, :]  # state augmented
        X_a_ctrl_u = X_augmented[3:6, :]  # control augmented
        X_a_msmt_z = X_augmented[6:8, :]  # measurement augmented

        assert (8, dim) == X_augmented.shape
        assert (dim, 1) == w_c.shape
        assert (dim, 1) == w_m.shape

        logger.debug('w_c:' + repr(w_c.T))
        logger.debug('w_m:' + repr(w_m.T))

        ##########################################################################
        # pass sigma points through the modion model and compute gaussian stats
        ##########################################################################
        # 7
        # estimate sigma points by applying the motion model
        #X_x_bel = g(ctrl_u + X_a_ctrl_u, X_a_state_x)
        X_x_bel = motion_model(X_a_ctrl_u, X_a_state_x, ctrl_u)

        # 8
        #mu_bel = sum(w_m X_x_bel)
        mu_bel = dot(X_x_bel, w_m)
        assert (3, 1) == mu_bel.shape
        logger.debug('mu_bel:' + repr(mu_bel))

        # 9
        # Sigma_bel = sum(w_c * (X_x_bel - mu_bel) * (X_x_bel - mu_bel).T)
        X_minus_mu = X_x_bel - mu_bel
        Sigma_bel = dot(w_c.T * (X_minus_mu), (X_minus_mu).T)
        assert (3, 3) == Sigma_bel.shape

        ##########################################################################
        # predict observations at sigma points and compute Gaussian statistics
        ##########################################################################
        # 10
        # measurement update
        #Z_bel = h(X_x_bel) * X_a_msmt_z
        Z_bel = self._measurement_model(X_x_bel, X_a_msmt_z)

        # 11
        # mean of predicted measurement
        z_bel = dot(Z_bel, w_m)
        assert (2, 1) == z_bel.shape

        # 12
        # covariance of predicted measurement
        # S = sum(w_c * (X_x_bel - mu_bel))
        S = dot(w_c.T * (Z_bel - z_bel), (Z_bel - z_bel).T)
        assert (2, 2) == S.shape

        # 13
        # cross covariance between robot location and ovservation
        Sigma_x_z = dot(w_c.T * (X_x_bel - mu_bel), (Z_bel - z_bel).T)
        # TODO is this correct? why 3x2?
        assert (3, 2) == Sigma_x_z.shape

        ##########################################################################
        # update mean and cov
        ##########################################################################
        # 14
        # calculate the Kalman gain
        K = dot(Sigma_x_z, pinv(S))

        # 15
        # mu measurement update
        mu = mu_bel + dot(K, (measurement_z - z_bel))
        assert (3, 1) == mu.shape

        # 16
        # Sigma measurement update
        Sigma = Sigma_bel - dot(dot(K, S), K.T)
        assert (3, 3) == Sigma.shape

        # 17
        # TODO add multi var gauss here
        p = None

        # 18
        self.mu_old = mu
        self.Sigma_old = Sigma
        self.p = p


##############################################################################
if __name__ == '__main__':

    #########################################
    # init UKF
    #########################################
    real_pos = array([[-100., 0., 0]]).T
    assert (3, 1) == real_pos.shape

    Sigma = array([
            [1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0]
            ])
    ctrl_u = control(10., 0)
    measurement_z = array([[400], [0.]])

    ukf = UKF(real_pos, Sigma, World)

    #########################################
    # PYGAME
    #########################################
    # init pydame stuff to display everything
    window_size = (640, 480)
    pygame.init()
    window = pygame.display.set_mode(window_size)

    # set repeat interval for pressed keys
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
                    #ctrl_u.y = 5 * random.random()
                    measurement_z[0, 0] -= ctrl_u.x
                    #measurement_z[1, 0] += ctrl_u.y
                    real_pos[0, 0] += ctrl_u.x
                    #real_pos[1, 0] += ctrl_u.y

                    # execute kf
                    ukf.execute(ctrl_u=ctrl_u, measurement_z=measurement_z)

                    # print ground truth
                    print 'measurement'
                    print measurement_z

                    print 'real pos (black)'
                    print real_pos
                    draw_pose(real_pos)

                    print 'ctrl:'
                    print ctrl_u.x, ctrl_u.y

                    # visualize stuff
                    visualize_kf(ukf)
                    visualize_measurement(measurement_z, ukf.mu_old)

                    print '=' * 78

                    # update screen
                    window.blit(pitch_surface, pitch_pos)
                    pygame.display.update()
