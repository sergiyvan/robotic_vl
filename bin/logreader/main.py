#!/usr/bin/env python

"""
Read the pbl logfiles and visualize them.

To extend this simply
- write a Visualizer for the proto extension (eg like
  the CircleAreaViz(Visualization))
- register the proto extension and the visualizer at the extension_registry
- and you are done.
"""



import os
import sys
# add install/ to be able to import the proto files
protobuf_path = os.path.realpath(sys.path[0] + "/../../install")
sys.path.insert(0, protobuf_path)
# add our proto version to the path. we don't need protobuf to be installed
protobuf_path = os.path.realpath(sys.path[0] + "/../tools/protobuf/pyshared")
sys.path.insert(0, protobuf_path)

from collections import OrderedDict
from datetime import datetime

from logfilereader import LogFileReader

# import all protobuf files (see __init__.py) to automaticaly print all
# extensions
#import protobuf
from protobuf import crf_modulelog_pb2

from protobuf import crf_ball_pb2
from protobuf import crf_circlearea_pb2
from protobuf import crf_edge_pb2
from protobuf import crf_frameInfo_pb2
from protobuf import crf_fieldcontour_pb2
from protobuf import crf_fieldlinefeatures_pb2
from protobuf import crf_goal_pb2
from protobuf import crf_pole_pb2
from protobuf import msg_image_pb2
from protobuf import crf_groundtruth_pb2

import sys
#import and init pygame
import pygame

from pygame.locals import *
from pygame.locals import KEYDOWN
from pygame.locals import KEYDOWN
from pygame.locals import K_ESCAPE
#from pygame.locals import K_RETURN
from pygame.locals import K_RIGHT
from pygame.locals import K_LEFT
from pygame.locals import K_QUESTION
from pygame.locals import K_h
from pygame.locals import K_i
from pygame.locals import K_n
from pygame.locals import K_p
from pygame.locals import K_q


WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (200, 200, 200)
RED = (255, 0, 0)
YELLOW = (255, 255, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)



##############################################################################
def draw_rect(rectangle, color, fill=0, surface=None):
    """ helper function to draw rectangles """
    if not surface:
        surface = window
    r = (rectangle.x, rectangle.y, rectangle.width, rectangle.height)
    pygame.draw.rect(surface, color, r, fill)


def draw_text(txt, pos, color, surface):
    """ helper function to draw some text """
    text = font.render(txt, 1, color)
    surface.blit(text, pos)


##############################################################################
class Visualization:
    """ Every datatype needs to have a Visualization. """
    color = None
    name = "None"
    key = None
    active = True

    @classmethod
    def draw(cls, data, surface):
        raise NotImplementedError()


class FieldLineFeaturesViz(Visualization):
    color = RED
    name = 'FieldLineFeatures'
    key = K_k

    @classmethod
    def draw(cls, data, surface):
        for feature in data:
            if feature.type == 1:
                #X
                left = feature.imagePos.x - 5, feature.imagePos.y
                right = feature.imagePos.x + 5, feature.imagePos.y
                top = feature.imagePos.x, feature.imagePos.y - 5
                bottom = feature.imagePos.x, feature.imagePos.y + 5
            elif feature.type == 2:
                # T
                left = feature.imagePos.x - 5, feature.imagePos.y
                right = feature.imagePos.x + 5, feature.imagePos.y
                top = feature.imagePos.x, feature.imagePos.y
                bottom = feature.imagePos.x, feature.imagePos.y -10
            elif feature.type == 3:
                # L
                left = feature.imagePos.x, feature.imagePos.y
                right = feature.imagePos.x, feature.imagePos.y + 10
                top = feature.imagePos.x -10, feature.imagePos.y
                bottom = left
            else:
                # ?
                # this should not happen :)
                left = feature.imagePos.x, feature.imagePos.y
                right = feature.imagePos.x, feature.imagePos.y
                top = feature.imagePos.x, feature.imagePos.y
                bottom = feature.imagePos.x, feature.imagePos.y

            pygame.draw.line(surface, cls.color, left, right)
            pygame.draw.line(surface, cls.color, top, bottom)


class ImageViz(Visualization):
    color = BLACK
    name = 'ImageViz'

    @classmethod
    def draw(cls, data, surface):
        pass


class CircleAreaViz(Visualization):
    color = BLACK
    name = 'circlearea'
    key = K_g

    @classmethod
    def draw(cls, data, surface):
        #pos = (data.centerX, data.centerY)
        #pygame.draw.circle(surface, cls.color, pos, data.radius)
        pass


class FieldContourViz(Visualization):
    color = BLUE
    name = 'fieldcontour'
    key = K_c

    @classmethod
    def draw(cls, data, surface):
        for x, y in enumerate(data.yContour):
            pygame.draw.circle(surface, cls.color, (x, y), 0)


class BallViz(Visualization):
    color = RED
    name = 'ball'
    key = K_b

    @classmethod
    def draw(cls, data, surface):
        if data.valid:
            draw_rect(data.rectangle, cls.color)


class ImageEdgesViz(Visualization):
    color = GRAY
    name = 'imageedges'
    key = K_i

    @classmethod
    def draw(cls, data, surface):
        for edge in data:
            old_point = edge.edgePoints[0]
            old_pos = old_point.pos.x, old_point.pos.y
            for p in edge.edgePoints:
                pos = (p.pos.x, p.pos.y)
                pygame.draw.line(surface, GRAY, old_pos, pos)
                old_pos = pos


class FieldLineEdgesViz(Visualization):
    color = BLACK
    name = 'fieldlineedges'
    key = K_f

    @classmethod
    def draw(cls, data, surface):
        for edge in data:
            old_point = edge.edgePoints[0]
            old_pos = old_point.pos.x, old_point.pos.y
            for p in edge.edgePoints:
                pos = (p.pos.x, p.pos.y)
                pygame.draw.line(surface, cls.color, old_pos, pos)
                old_pos = pos


class GoalBlueViz(Visualization):
    color = BLUE
    name = 'goal blue'

    @classmethod
    def draw(cls, data, surface):
        for pole in [data.leftPole, data.rightPole]:
            if pole.valid:
                draw_rect(pole.rectangle, cls.color)


class GoalYellowViz(GoalBlueViz):
    color = YELLOW
    name = 'goal yellow'


class GroundTruthViz(Visualization):
    color = BLACK
    name = 'ground thruth data'

    @classmethod
    def draw(cls, data, surface):
        robot = data.robotPoses[0]

        pos_txt = p2s((robot.pose.translation.x-10, robot.pose.translation.y-10))
        draw_text(str(robot.robotID), pos_txt, cls.color, pitch_surface)

        pos = p2s((robot.pose.translation.x, robot.pose.translation.y))
        pygame.draw.circle(pitch_surface, BLACK, pos, 5)
        # TODO draw direction
        #pygame.draw.line(pitch_surface, BLACK, pos, direction, 5)


class FrameInfoViz(Visualization):
    color = GRAY
    name = 'framinfo'

    @classmethod
    def draw(cls, data, surface):
        frametxt = 'frame: ' + str(data.frameNumber)
        frametime = str(datetime.fromtimestamp(data.frameStartTime / 1000))
        draw_text(frametime, (5, 5), GRAY, surface)
        draw_text(frametxt, (5, 15), GRAY, surface)


##############################################################################

# register all proto extensions with the corresponding visualizer.
# the order of the registratio of the visualizers matters because they draw
# on top of the surface.
extension_registry = OrderedDict([
        (msg_image_pb2.imageRepr, ImageViz),
        (crf_circlearea_pb2.circleArea, CircleAreaViz),
        (crf_fieldcontour_pb2.fieldContour, FieldContourViz),
        (crf_edge_pb2.imageEdges, ImageEdgesViz),
        (crf_edge_pb2.fieldLineEdges, FieldLineEdgesViz),
        (crf_fieldlinefeatures_pb2.fieldLineFeatures, FieldLineFeaturesViz),
        (crf_goal_pb2.blueGoal, GoalBlueViz),
        (crf_goal_pb2.yellowGoal, GoalYellowViz),
        (crf_ball_pb2.ballObject, BallViz),
        (crf_frameInfo_pb2.frameInfo, FrameInfoViz),
        (crf_groundtruth_pb2.groundTruthData, GroundTruthViz),
])

# mapping: key -> visualizer
# used to activate/deactivate the visualization
toggle_visualization_registry = {}
for visualizer in extension_registry.values():
    if visualizer.key:
        toggle_visualization_registry[visualizer.key] = visualizer


def visualize(protodata, window):
    """
    Visualize the given data
    """
    if not protodata:
        return
    # get every protoextension and executeiterate over every protoextension
    for protoextension in extension_registry:
        extensiondata = protodata.Extensions[protoextension]
        visualizer = extension_registry[protoextension]
        if visualizer.active:
            visualizer.draw(extensiondata, window)


##############################################################################
def update_legend(surface, size):
    """ update the legend """
    pos = 15
    pygame.draw.rect(surface, WHITE, (0, 0, size[0], size[1]))
    x_offset = 15
    draw_text('NAME            <togglekey>', (x_offset, pos), BLACK, surface)

    for visualizer in extension_registry.values():
        pos += 15
        visualizer.name
        if visualizer.key:
            txt = '{:<25} <{:>1}>'.format(
                    visualizer.name, pygame.key.name(visualizer.key))
        else:
            txt = visualizer.name
        draw_text(txt, (x_offset, pos), visualizer.color, surface)

        # draw active button
        r = (200, pos, 10, 10)
        if visualizer.active:
            pygame.draw.rect(surface, visualizer.color, r)
        else:
            pygame.draw.rect(surface, visualizer.color, r, 1)


##############################################################################
def update_pitch(surface, size):
    """ update the pitch """
    pygame.draw.rect(surface, GRAY, (0, 0, size[0], size[1]))
    pygame.draw.rect(surface, GREEN, (20, 40, 600, 400))

    # left goal line
    pygame.draw.line(surface, WHITE, (20, 40), (20, 440), 5)
    # middle line
    pygame.draw.line(surface, WHITE, (320, 40), (320, 440), 5)
    # right goal line
    pygame.draw.line(surface, WHITE, (620, 40), (620, 440), 5)
    # top line
    pygame.draw.line(surface, WHITE, (20, 40), (620, 40), 5)
    # bottom line
    pygame.draw.line(surface, WHITE, (20, 440), (620, 440), 5)
    # circle
    pygame.draw.circle(surface, WHITE, (320, 240), 60, 5)


def p2s(pos):
    """ translate pitch coordinates to surface coordinates """
    return 320 + pos[0], 240 - pos[1]


##############################################################################
def toggle_visualization(key):
        vis = toggle_visualization_registry[key]
        vis.active = not vis.active
        print 'Toggle ', vis.name,
        if vis.active:
            print ' -- it is active.'
        else:
            print ' -- it is inactive.'


##############################################################################
# this is not nice (but confenient :)). shouldn't be global
window = None
font = None
legend_surface = None
pitch_surface = None


def main(fname):
    global window
    global legend_surface
    global pitch_surface
    global font


    helptxt = """
    LEFTARROW/RIGHTARROW     previous/next frame
    p                        print proto data
    q/ESC                    quit
    h/?                      help
    """
    print helptxt

    logfilereader = LogFileReader(fname)

    # init pydame stuff to display everything
    window_size = (1280, 760)
    pygame.init()
    window = pygame.display.set_mode(window_size)

    # set repeat interval for pressed keys
    pygame.key.set_repeat(200, 5)

    pygame.display.flip()

    # random init
    data_area = (0, 0, 640, 480)
    font = pygame.font.Font(None, 16)

    # legend
    legend_pos = (0, 490)
    legend_size = (500, 400)
    legend_surface = pygame.Surface(legend_size)
    update_legend(legend_surface, legend_size)

    # pitch / soccer field
    pitch_pos = (641, 0)
    pitch_size = (640, 480)
    pitch_surface = pygame.Surface(pitch_size)

    # draw initial data
    data = logfilereader.get_next_frame()

    pygame.draw.rect(window, WHITE, data_area)
    update_pitch(pitch_surface, pitch_size)

    visualize(data, window)

    window.blit(pitch_surface, pitch_pos)
    window.blit(legend_surface, legend_pos)

    pygame.display.update()

    # main loop
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

            elif event.type == KEYDOWN:
                # QUIT
                if event.key == K_ESCAPE or event.key == K_q:
                    print "Aborting...wait a sec..."
                    done = True
                    break

                # process next frame
                elif event.key == K_RIGHT:
                    data = logfilereader.get_next_frame()

                # process previous frame
                elif event.key == K_LEFT:
                    data = logfilereader.get_previous_frame()

                # toggle different visualizations
                elif event.key in toggle_visualization_registry:
                    toggle_visualization(event.key)
                    update_legend(legend_surface, legend_size)

                # print all data
                elif event.key == K_p:
                    print data

                # help
                elif event.key == K_h or event.key == K_QUESTION:
                    print helptxt

                # visualize data on every keydown
                pygame.draw.rect(window, WHITE, data_area)
                update_pitch(pitch_surface, pitch_size)

                visualize(data, window)

                window.blit(pitch_surface, pitch_pos)
                window.blit(legend_surface, legend_pos)

                pygame.display.update()

    print "left loop"


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print "Please specify the path to a *.pbl."
        sys.exit()

    fname = sys.argv[1]
    if not '.pbl' in fname:
        print "Please specify the path to a *.pbl."
        sys.exit()

    main(fname)
    print "pygame quit"
