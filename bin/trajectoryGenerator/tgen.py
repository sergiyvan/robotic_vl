#!/usr/bin/python

"""
Tool that creates a csv fil of the walking trajectories.

This python script creates such files and is easier to update and maintain as a
csv file itself.

See Johannes Kulicks master thesis for more infos.
"""


from math import cos, sin, sqrt, atan2, pi
import csv


upperLeg   = 63
lowerLeg   = 63
kneeLength = 50
hipLength  = 40
footX      = 0
footZ      = 25


motornames = ['HEAD_TURN',
        'ARM_PITCH_LEFT',
        'ARM_PITCH_RIGHT',
        'ARM_ROLL_LEFT',
        'ARM_ROLL_RIGHT',
        'ELLBOW_LEFT',
        'ELLBOW_RIGHT',
        'HIP_YAW_LEFT',
        'HIP_YAW_RIGHT',
        'HIP_ROLL_LEFT',
        'HIP_ROLL_RIGHT',
        'HIP_PITCH_LEFT',
        'HIP_PITCH_RIGHT',
        'KNEE_TOP_LEFT',
        'KNEE_TOP_RIGHT',
        'KNEE_BOTTOM_LEFT',
        'KNEE_BOTTOM_RIGHT',
        'FOOT_PITCH_LEFT',
        'FOOT_PITCH_RIGHT',
        'FOOT_ROLL_LEFT',
        'FOOT_ROLL_RIGHT']


def getIntersection(x, z):
    x2  = x - footX
    y   = hipLength
    y2  = z - footZ - kneeLength

    m   = (- x2) / (y2 - y)
    b   = (- lowerLeg * lowerLeg + upperLeg*upperLeg + y2*y2 - y*y +
            x2*x2)/(2*(y2-y))

    p   = (2 * m * (b - y))/(1 + m*m)
    q   = ((b-y) * (b-y) - upperLeg * upperLeg)/(1 + m*m)

    sqr = sqrt((p * p/4) - q)

    sx  = (-p / 2 + sqr)
    sz  = (m * sx + b)

    return (sx, sz)


def inverseLegKinematic(x, y, z, yaw, foot, motorvalues):
    x2 = x * cos(yaw) + y * sin(yaw)
    y2 = x * sin(yaw) + y * cos(yaw)
    z2 = z

    x3 = x2
    y3 = y2
    z3 = sqrt(y2 * y2 + z2 * z2)

    (sx, sz) = getIntersection(x3, z3)

    motorvalues['HIP_YAW_' + foot]     = yaw
    motorvalues['HIP_ROLL_' + foot]    = atan2(y2, z2)
    motorvalues['KNEE_TOP_' + foot]    = -(pi/2 - (atan2(sz - hipLength, sx)))
    motorvalues['KNEE_BOTTOM_' + foot] = -(pi/2 - (atan2(z3 - footZ - sz - kneeLength,
        x3 - footX - sx)))
    motorvalues['FOOT_ROLL_' + foot]   = -motorvalues['HIP_ROLL_' + foot]

    return motorvalues


def computeFootPosition(time, speed):
    x = sin(time*pi/180-pi/4) * (speed/2)
    z = 230 - (sin(time * pi/180) * 30)
    if (z > 230):
        z = 230
    return (x, z)


def main():
    # close file automatically
    with open('motorvalues.csv','wb') as file_:
        motorWriter = csv.DictWriter(file_, motornames)
        motorWriter.writeheader()
        for time in range(360):
            # reinitialize mv
            mv = dict(zip(motornames, [0] * len(motornames)))

            (x_left,  z_left)  = computeFootPosition(time, 50)
            (x_right, z_right) = computeFootPosition(time-180, 50)

            mv = inverseLegKinematic(x_left, 0, z_left, 0, 'LEFT', mv)
            mv = inverseLegKinematic(x_right, 0, z_right, 0, 'RIGHT', mv)

            motorWriter.writerow(mv)
    print 'motorvalues.csv has been written to current working dir.'
    print 'DONE'


if __name__ == "__main__":
    main()
