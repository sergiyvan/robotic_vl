import numpy as np

from scipy import stats

from helper_math import normalize_radians
from helper_math import Pose2D


class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y


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
    BEACON = Point(300, 0)

    # goal yellow
    y1 = Point(300, 75)
    y2 = Point(300, -75)

    # goal blue
    b1 = Point(-300, 75)
    b2 = Point(-300, -75)

    yby = Point(0, 240)
    byb = Point(0, -240)

    x_features = [Point(0, 60), Point(0, -60)]

    t_features = [Point(*p) for p in [
                    (0, -200), (0, 200),
                    (-300, -150), (-300, 150),
                    (300, -150), (300, 150)
            ]]

    l_features = [Point(*p) for p in [
                    (-300, -200), (-300, 200),  # field corners
                    (300, -200), (300, 200),  # field corners
                    (-240, -150), (-240, 150),  # goal corners
                    (240, -150), (240, 150)  # goal corners
            ]]

    circle = Point(0, 0)

    fieldline_features = []
    fieldline_features.extend(x_features)
    fieldline_features.extend(t_features)
    fieldline_features.extend(l_features)


def p2s(pos):
    """
    pitch to surface.

    translate pitch coordinates to surface coordinates
    """
    return 320 + int(pos[0]), 240 - int(pos[1])


def world2measurement(robot, point):
    """
    Given the pose of the robot and a point in the world, compute the
    observation in polar coordinates.

    >>> robot = Pose2D(0., 0., 0.)
    >>> pnt = Point(100., 0.)
    >>> world2measurement(robot, pnt)
    array([[ 100.],
           [   0.]])

    >>> robot = Pose2D(50., 0., 0.)
    >>> pnt = Point(100., 0.)
    >>> world2measurement(robot, pnt)
    array([[ 50.],
           [  0.]])

    >>> robot = Pose2D(0., 0., 0.)
    >>> pnt = Point(0., 100.)
    >>> world2measurement(robot, pnt)
    array([[ 100.        ],
           [   1.57079633]])

    >>> robot = Pose2D(100., 0., 0.)
    >>> pnt = Point(100., 100.)
    >>> world2measurement(robot, pnt)
    array([[ 100.        ],
           [   1.57079633]])

    >>> robot = Pose2D(100., 0., np.pi / 2)
    >>> pnt = Point(100., 100.)
    >>> world2measurement(robot, pnt)
    array([[ 100.],
           [   0.]])

    >>> robot = Pose2D(100., 0., - np.pi / 2)
    >>> pnt = Point(100., 100.)
    >>> world2measurement(robot, pnt)
    array([[ 100.        ],
           [  -3.14159265]])

    >>> robot = Pose2D(0., 0., 0.)
    >>> pnt = Point(100., 100.)
    >>> world2measurement(robot, pnt)
    array([[ 141.42135624],
           [   0.78539816]])
    """
    result = np.array([
                [np.sqrt((point.x - robot.x) ** 2 + (point.y - robot.y) ** 2)],
                [normalize_radians(
                    np.arctan2(point.y - robot.y, point.x - robot.x) - robot.rot)
                ]])
    assert (2, 1) == result.shape

    return result


def prob_of_correspondence(observation, world_point, robot):
    """
    given the prob for an observation given the corresponding point in the
    world and the robot (Pose2D).

    NOTE: Not normalized!

    P(observation | robot, world_point)

    See page. 179

    Unittest depend on noise parameters. Noise parameters change often,
    therefore the tests are disabled.

    a perfect match:
    #>>> observation = np.array([[100.], [0]])
    #>>> world_point = Point(100., 0.)
    #>>> robot = Pose2D(0., 0., 0.)
    #>>> prob(observation, world_point, robot)
    0.15915494309189535

    bearing slightly off:
    #>>> observation = np.array([[100.], [0.2]])
    #>>> world_point = Point(100., 0.)
    #>>> robot = Pose2D(0., 0., 0.)
    #>>> prob(observation, world_point, robot)

    #True

    #too far away:
    #>>> observation = np.array([[100.], [0]])
    #>>> world_point = Point(200., 0.)
    #>>> robot = Pose2D(0., 0., 0.)
    #>>> prob(observation, world_point, robot) < 0.1

    #True

    #bearing WAY off:
    #>>> observation = np.array([[100.], [0]])
    #>>> world_point = Point(100., 0.)
    #>>> robot = Pose2D(0., 0., 3.2)
    #>>> prob(observation, world_point, robot)
    #>>> prob(observation, world_point, robot) < 0.15915494309189535

    #True
    """
    NOISE_DIST = 20.
    NOISE_BEAR = .3

    expected_distance = np.sqrt((world_point.x - robot.x) ** 2
               + (world_point.y - robot.y) ** 2)
    expected_angle = normalize_radians(np.arctan2(world_point.y - robot.y,
                                                  world_point.x - robot.x)
                                       + robot.rot)

    #print 'observation', observation[0, 0], observation[1, 0]
    #print 'expected   ', expected_distance, expected_angle

    res = stats.norm.pdf(expected_distance, observation[0, 0], NOISE_DIST) * \
          stats.norm.pdf(expected_angle, observation[1, 0], NOISE_BEAR)

    return res


def test():
    """
    test selection of feature
    """
    observation = np.array([[150.], [np.pi / 2]])
    robot = Pose2D(0., 0., np.pi)

    print 'observation at', observation[0, 0], observation[1, 0]

    results = []
    print "X" + '-' * 60
    for world_point in World.x_features:
        p = prob_of_correspondence(observation, world_point, robot)
        results.append(p)
        print 'feature %d/%d prob %f' % (world_point.x, world_point.y, p)

    print "T" + '-' * 60
    for world_point in World.t_features:
        p = prob_of_correspondence(observation, world_point, robot)
        results.append(p)
        print 'feature %d/%d prob %f' % (world_point.x, world_point.y, p)

    print "L" + '-' * 60
    for world_point in World.l_features:
        p = prob_of_correspondence(observation, world_point, robot)
        results.append(p)
        print 'feature %d/%d prob %f' % (world_point.x, world_point.y, p)

    sum_ = sum(results)
    for item in results:
        print ('%f' % (item / sum_).round(4))

    #for world_point in World.fieldline_features:

if __name__ == '__main__':
    test()
