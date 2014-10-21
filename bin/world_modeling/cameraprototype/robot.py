from math import cos, sin, radians
import numpy as np
from camera import Camera
from world import World

class Robot:
    """Models a FUmanoid"""

    def __init__(self):
        # position vector, should have 3d, last 0
        self.position = np.zeros((3,1))
        # the yaw angle of the whole robot
        self.orientation = 0
        self.camera = Camera()
        self.gyro_matrix = np.identity(3)
        self.world = World()
    
    def set_pitch(self, pitch):
        y = self.gyro_matrix[1,:]
        z = np.array([cos(radians(pitch)), 0, sin(radians(pitch))])
        x = np.cross(z, y)
        self.gyro_matrix = ([x,y,z])

    def set_roll(self, roll):
        y = np.array([0, cos(radians(roll)), sin(radians(roll))]);
        z = self.gyro_matrix[2,:]
        x = np.cross(z, y);
        self.gyro_matrix = ([x,y,z])
        
    def to_homogenous(self, x):
        return np.append(x,[[1]],0)

    def get_world_to_robot_matrix(self):
        rotation_matrix = np.array(
            [[cos(radians(self.orientation)), -sin(radians(self.orientation)), 0],
            [sin(radians(self.orientation)), cos(radians(self.orientation)), 0], 
            [0, 0, 1]])
        rotation_matrix_transposed = rotation_matrix.T; 
        rotation_matrix_dot_translation = -1*np.dot(rotation_matrix_transposed,
                self.position)
        world_to_robot_matrix = np.append(rotation_matrix_transposed,
                rotation_matrix_dot_translation,axis=1)
        return world_to_robot_matrix 

    def get_robot_to_head_matrix(self):
        robot_height = 50;
        robot_vector = np.array([[0],[0],[robot_height]])
        rotation_matrix_transposed = self.gyro_matrix.T; 
        rotation_matrix_dot_translation = -1*np.dot(rotation_matrix_transposed,
                robot_vector)
        robot_to_head_matrix = np.append(rotation_matrix_transposed,
                rotation_matrix_dot_translation,axis=1)
        return robot_to_head_matrix 

    def get_camera_image(self):

        scale = 1

        img = np.zeros((480,640))
        w2r = self.get_world_to_robot_matrix()
        r2h = self.get_robot_to_head_matrix()
        h2i = self.camera.get_head_to_image_matrix()
        for line in self.world.lines:
            color = line.get_color()
            for point in line.get_points(scale):
                robot_point = np.dot(w2r, self.to_homogenous(point))
                head_point = np.dot( r2h, self.to_homogenous(robot_point))
                img_p = np.dot(h2i, head_point)
                if img_p[2] <= 0:
                    continue
                img_p = img_p[0:2,:]/img_p[2,:]
                if img_p[1] < 639 and img_p[1] > 1 and img_p[0] < 479 and img_p[0] > 1 :
                    paint_point(img, img_p, color)
        return img

    def get_image_lines(self):
        # TODO
        image_lines = []
        w2r = self.get_world_to_robot_matrix()
        r2h = self.get_robot_to_head_matrix()
        r2i = self.camera.get_robot_to_image_matrix()
        for line in self.world.lines:
            robot_start = np.dot(w2r, self.to_homogenous(line.start.v))
            head_start = np.dot( r2h, self.to_homogenous(robot_start))
            img_p = np.dot(r2i, robot_start)
            if img_p[2] <= 0:
                continue
            img_p = self.camera.robot_point_to_image_point(head_start)
            img_p = img_p[0:1,:]/img_p[2,0]
    
def paint_point(img, img_p, color):
    img[int(img_p[0]),int(img_p[1])] = color
    img[int(img_p[0])+1,int(img_p[1])+1] = color
    img[int(img_p[0])-1,int(img_p[1])-1] = color
    img[int(img_p[0])+1,int(img_p[1])+1] = color
    img[int(img_p[0])-1,int(img_p[1])-1] = color
    img[int(img_p[0]),int(img_p[1])+1] = color
    img[int(img_p[0]),int(img_p[1])-1] = color
    img[int(img_p[0])+1,int(img_p[1])] = color
    img[int(img_p[0])-1,int(img_p[1])] = color

