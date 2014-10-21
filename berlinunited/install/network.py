import gobject
import socket
import time
from threading import Thread
import struct


try:
    import pygtk
    import gtk
    pygtk.require("2.0")
except:
    pass


####################################################################################################


port = 11011

####################################################################################################

class MessageSender():
    """
    MessageSender is responsible for creating and sending messages to the robots.

    For more information about the protocol take a look at doc/comm.txt
    """

    def __init__(self):
        self.outgoingSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.outgoingSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.outgoingSocket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.outgoingSocket.settimeout(1.0)

    def group(self, groupMembers):
        data = ''

        for (id, ip) in groupMembers.items():
            data = data + struct.pack("!B", id)

        for (id, ip) in groupMembers.items():
            self.sendMessage(20, 0, ip, data)

    def walk(self, forwardSpeed, sidewardSpeed, rotationSpeed, ip):
        """Walk (OP 13)"""
        data = ''
        data += struct.pack("!b", forwardSpeed)
        data += struct.pack("!b", sidewardSpeed)
        data += struct.pack("!b", rotationSpeed)
        self.sendMessage(13, 0, ip, data)

    def buttonPressed(self, buttonNumber, ip):
        """Send the button pressed command (OP 31)"""
        data = ''
        data += struct.pack("!b", buttonNumber)
        self.sendMessage(31, 0, ip, data)

    def startGame(self, ip):
        data = ''
        self.sendMessage(11, 0, ip, data)

    def startRole(self, roleID, ip):
        """Start the role with the given roleID"""
        data = ''
        data += struct.pack("!c", roleID[0])
        data += struct.pack("!c", roleID[1])
        data += struct.pack("!c", roleID[2])
        data += struct.pack("!c", roleID[3])
        self.sendMessage(3, 0, ip, data)

    def sendMessage(self, op, flags, ip, data):
        """send the message to the robot"""
        msg = struct.pack("!HBB", op, flags, 0) + data
        self.outgoingSocket.sendto(msg, (ip, port))


####################################################################################################

class MessageListener(Thread):
    def __init__(self, robotList, robotLabels):
        Thread.__init__(self)

        self.robotList = robotList
        self.robotLabels = robotLabels

        self.connected = False
        self.running = False

        self.groupedTeam = {}

        self.lastSeen = {}
        for (id, data) in self.robotLabels.items():
            self.lastSeen[id] = 1.0

    def __del__(self):
        self.disconnect()

    def setGroupedTeam(self, groupedTeam):
        for (id, data) in self.robotLabels.items():
            grouped = groupedTeam.get(id, None)
            if (grouped == None):
                data["name"].set_text( data["nameValue"] )
            else:
                data["name"].set_text( data["nameValue"] + "*" )

        self.groupedTeam = groupedTeam

    def connect(self):
        if self.connected == True:
            return

        try:
            self.incomingSocket = socket.socket(socket.AF_INET,
                    socket.SOCK_DGRAM)
            self.incomingSocket.setsockopt(socket.SOL_SOCKET,
                    socket.SO_REUSEADDR, 1)
            self.incomingSocket.setsockopt(socket.SOL_SOCKET,
                    socket.SO_BROADCAST, 1)
            self.incomingSocket.settimeout(1.0)
            print port
            self.incomingSocket.bind(('', port))
            self.connected = True
            self.start()
        except Exception, message:
            print Exception
            print message
            msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR,
                    gtk.BUTTONS_OK,
                    "Error binding to port. Is FUremote running?")
            res = msg.run()
            msg.destroy()
            return False

        gobject.timeout_add(500,   self.updateTeam)
        return True

    def disconnect(self):
        if (self.running == True):
            self.running = False
            self.join(1)
            self.incomingSocket.close()

    def run(self):
        self.running = True
        while self.running == True:
            try:
                message, address = self.incomingSocket.recvfrom(64)
                (op, flags) = struct.unpack_from("!HB", message)
                if op == 1:
                    (id, orientation, strategyID, roleID, behaviorID,
                            motionID, posX, posY, ballX, ballY, battery) = \
                            struct.unpack_from('!BxhIIIIhhhhB', message, 4)

                    strategy = role = behavior = motion = "-"
                    if strategyID != 0: (strategy,) = struct.unpack_from(
                            '!4s', message, 7)
                    if roleID != 0: (role,) = struct.unpack_from(
                            '!4s', message, 11)
                    if behaviorID != 0: (behavior,) = struct.unpack_from(
                            '!4s', message, 15)
                    if motionID != 0: (motion,) = struct.unpack_from(
                            '!4s', message, 19)

                    self.update(id, strategy, role, behavior, motion, posX,
                            posY, ballX, ballY, battery)

                elif op == 21 and flags == 0:
                    secs = time.time()
                    if time.daylight:
                        secs = secs - time.altzone
                    else:
                        secs = secs - time.timezone

                    msg = struct.pack("!HBBI", 21, 1, 0, int(secs))
                    self.incomingSocket.sendto(msg, address)

            except socket.timeout:
                pass
            except (KeyboardInterrupt, SystemExit):
                self.running = False
                raise


    def update(self, id, strategy, role, behavior, motion, posX, posY, ballX, ballY, battery):
        data = self.robotLabels.get(id, None)
        if data != None:
            if (battery <= 100):
                data["battery"] .set_text("%d%%" % battery)
            else:
                data["battery"] .set_text("???");

            #data["strategy"].set_text("%s" % strategy)
            #data["role"]    .set_text("%s" % role)
            #data["behavior"].set_text("%s" % behavior)
            #data["motion"]  .set_text("%s" % motion)
            data["position"].set_text("(%d,%d)" % (posX,  posY))
            data["ballpos"] .set_text("(%d,%d)" % (ballX, ballY))
            self.lastSeen[id] = time.time()


    def updateTeam(self):
        for (id, data) in self.robotLabels.items():
            if self.lastSeen[id] != 0 and self.lastSeen[id] + 2 < time.time():
                for (labelName, label) in data.items():
                    if (labelName != "name" and labelName != "nameValue"):
                        label.set_text("-")
                self.lastSeen[id] = 0

        return True
