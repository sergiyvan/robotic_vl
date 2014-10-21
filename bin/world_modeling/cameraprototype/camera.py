from math import cos, sin, radians
import numpy as np

class Camera:
    """Models a camera"""

    def __init__(self):
        self.yaw = 0
        self.pitch = 130
        self.internal_matrix = np.identity(3)
        self.read_config("./cam.cfg")

    def get_head_to_image_matrix(self):
        """calculates the matrix transfroming robot points to image points"""
        rotation_matrix = self.build_rotation_matrix();
        rotation_matrix_transposed = rotation_matrix.T; 
        camera_matrix = np.dot(self.internal_matrix, rotation_matrix_transposed)
        return camera_matrix 

    def build_rotation_matrix(self):
        r1 = np.array(
            [[cos(radians(self.yaw)), -sin(radians(self.yaw)), 0],
            [sin(radians(self.yaw)), cos(radians(self.yaw)), 0], 
            [0, 0, 1]])
        r2 = np.array(
            [[cos(radians(self.pitch)), 0, sin(radians(self.pitch))],
            [0, 1, 0],
            [-sin(radians(self.pitch)), 0, cos(radians(self.pitch))]])
        return np.dot(r2,r1)

    def head_point_to_image_point(self, head_point):
        image_point_3d = np.dot(self.get_head_to_image_matrix(), head_point)
        return image_point_3d[0:2,:]/image_point_3d[2,:]

    def read_config(self, fileName):
        self.internal_matrix = np.diag([0,0,1])
        with open(fileName, 'r') as f:
            for line in f:
                cfg_pair = line.rpartition('=')
                if cfg_pair[0].strip() == 'ALPHA_X':
                    self.internal_matrix[0,0] = int(cfg_pair[2].strip())
                if cfg_pair[0].strip() == 'ALPHA_Y':
                    self.internal_matrix[1,1] = float(cfg_pair[2].strip())
                if cfg_pair[0].strip() == 'SKEW':
                    self.internal_matrix[0,1] = float(cfg_pair[2].strip())
                if cfg_pair[0].strip() == 'OFFSET_X':
                    self.internal_matrix[0,2] = float(cfg_pair[2].strip())
                if cfg_pair[0].strip() == 'OFFSET_Y':
                    self.internal_matrix[1,2] = float(cfg_pair[2].strip())

    def __str__(self):
        s = ''
        s += 'Rotation Matrix' 
        s += '\n' + str(self.build_rotation_matrix())
        s += '\n' + 'Internal Parameter Matrix'
        s += '\n' + str(self.internal_matrix)
        s += '\n' + 'Complete Transformation Matrix'
        s += '\n' + str(self.get_robot_to_image_matrix())
        return s
