#! /usr/bin/python
from robot import Robot
from pylab import imshow, show, imsave
import numpy as np

def test():
    robot = Robot()
    mode = raw_input( "Enter mode: walk (1) or position (2):" )
    mode = int(mode)
    if mode == 1:
        robot.orientation = 0
        robot.position = np.array([[-300],[-200],[0]])
        for x in range(15):
            robot.position += np.array([[5],[5],[0]])
            robot.orientation += 1
            img = robot.get_camera_image()
            if x < 10:
                imsave("./images/0{}.png".format(x), img,0,5)
            else:
                imsave("./images/{}.png".format(x), img,0,5)
        for x in range(15,25):
            robot.position += np.array([[10],[0],[0]])
            robot.orientation -= 2
            img = robot.get_camera_image()
            imsave("./images/{}.png".format(x), img,0,5)
        for x in range(25,35):
            robot.position += np.array([[3],[7],[0]])
            robot.orientation += 2
            img = robot.get_camera_image()
            imsave("./images/{}.png".format(x), img,0,5)

    if mode == 2:
        input_string = raw_input( "Enter orientation in degrees:" )
        robot.orientation = float(input_string)
        input_string = raw_input( "Enter x coordinate:" )
        x = float(input_string)
        input_string = raw_input( "Enter y coordinate:" )
        y = float(input_string)
        robot.position = np.array([[x],[y],[0]])
        img = robot.get_camera_image()
        imsave("./images/position_img.png", img,0,5)

    if mode == 3:
        robot.orientation = 0
        robot.position = np.array([[-290],[0],[0]])
        img = robot.get_camera_image()
        imsave("./images/position_img.png", img,0,5)

if __name__ == '__main__':
    test()
