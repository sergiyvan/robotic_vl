from point import Point
from line import Line
from circle import Circle

class World:
    """Models a soccer field fo RoboCup"""
    def __init__(self):
        self.lines = [ 
            # This is the middle line
            Line(Point(0,0),Point(0, 200)),
            Line(Point(0,0), Point(0, -200)),
            # These are the outer field lines
            Line(Point(-300, -200), Point(300, -200)),
            Line(Point(-300, 200),Point(300, 200)),
            Line(Point(-300, -200), Point(-300, 200)),
            Line(Point(300, -200),Point(300, 200)),
            # These are the goal areas
            Line(Point(-300,150),Point(-240, 150)),
            Line(Point(-300,-150),Point(-240, -150)),
            Line(Point(-240,150),Point(-240,-150)),
            Line(Point(300,150),Point(240, 150)),
            Line(Point(300,-150),Point(240, -150)),
            Line(Point(240,150),Point(240,-150)),
            # These are the goals
            Line(Point(300,75,0,2),Point(300,75,80,2)),
            Line(Point(300,-75,0,2),Point(300,-75,80,2)),
            Line(Point(300,75,80,2),Point(300,-75,80,2)),
            Line(Point(-300,75,0,3),Point(-300,75,80,3)),
            Line(Point(-300,-75,0,3),Point(-300,-75,80,3)),
            Line(Point(-300,75,80,3),Point(-300,-75,80,3)),
            # These are the poles
            Line(Point(0,240,45,4),Point(0,240,0,4)),
            Line(Point(0,-240,0,4),Point(0,-240,45,4)),
            # This is the circle
            Circle(Point(0,0),60)]
