import os
import re
import sys
import time
import struct
import gobject
import socket
import zlib

from threading import Thread, Event
from protobuf import msg_image_pb2
from protobuf import msg_message_pb2

try:
    sys.path.append("/usr/local/opencv/lib/python2.6/site-packages")
    import cv
except:
    pass

try:
    import pygtk
    import gtk
    pygtk.require("2.0")
except:
    pass



def setComboboxValues(cb, items):
    """Setup a ComboBox or ComboBoxEntry based on a list of strings."""
    model = gtk.ListStore(str)
    for i in items:
        model.append([i])
    cb.set_model(model)
    if type(cb) == gtk.ComboBoxEntry:
        cb.set_text_column(0)
    elif type(cb) == gtk.ComboBox:
        cell = gtk.CellRendererText()
        cb.pack_start(cell, True)
        cb.add_attribute(cell, 'text', 0)


class CameraImageUpdater(Thread):
    def __init__(self, gui, event, robotIP, cameraImage):
        Thread.__init__(self)

        self.gui         = gui
        self.event       = event
        self.robotIP     = robotIP
        self.cameraImage = cameraImage
        self.saveStream  = False

        self.originalcvImage = 0

    '''
    Try to read 'size' bytes from the socket
    '''
    def readData(self, sock, requestedSize, stats):
        dataSize = 0
        data     = []

        while dataSize < requestedSize:
            data.append( sock.recv( min(2048, requestedSize - dataSize)) )
            dataSize = sum([len(i) for i in data ])

            if stats:
                self.gui.updateActivity("Receiving image %d%%" % (dataSize*100/requestedSize))

        return '' . join(data)

    def getImageType(self):
        return self.gui.overlayCbx.get_active() + 1

    def getScale(self):
        index = self.gui.scalingCbx.get_active()

        if index < 0:
            return 0
        else:
            return 2 ** index - 1

    def run(self):
        self.running = True
        while self.running == True:
            self.event.wait()

            try:
                self.gui.updateActivity("Connecting to robot ...")

                # Create a socket (SOCK_STREAM means a TCP socket)
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

                # Connect to server
                sock.connect((self.robotIP, 11011))

                # send request
                self.gui.updateActivity("Requesting image ...")

                msg = msg_message_pb2.Message()
                msg.robotID = 1

                req = msg.Extensions[msg_image_pb2.imageRequest]
                req.type  = msg_image_pb2.RGB_IMAGE;
                req.count = 1
                req.includeRGB = True
                req.includeRAW = False
                req.type = self.getImageType()
                req.scaling = self.getScale()

                data = msg.SerializeToString()
                header = struct.pack("!HbbI", 42, 0, 0, len(data));

                sock.send(header)
                sock.send(data)

                # read return header
                (msgType, flags, unused, dummy, msgSize) = struct.unpack("!HbbII", self.readData(sock, 12, False))
                updateTimestamp = time.strftime("%H:%M:%S")
                print "got msg " + str(msgType) + " with size " + str(msgSize)

                # actually read data
                serializedMsg = self.readData(sock, msgSize, True)
                msg = msg_message_pb2.Message()
                msg.ParseFromString(serializedMsg)

                if msg.HasExtension(msg_image_pb2.image):
                    for id in msg.Extensions[msg_image_pb2.image].imageData:
                        if id.format == msg_image_pb2.RGB_IMAGE:
                            self.showImage(id, updateTimestamp)

                # we do not need the socket anymore
                sock.close()

            except socket.timeout:
                pass
            except (KeyboardInterrupt, SystemExit):
                self.running = False
                raise
#            except Exception, message:
#                print Exception
#                print message
#                sock.close()
#                self.gui.updateActivity("Image transfer failed")
#                pass

            self.gui.lastImageTime = time.time()
            self.event.clear()

        print "Thread finished"

    def showImage(self, imageData, updateTimestamp):
        # if necessary decompress
#        if compressedSize < uncompressedSize:
#            imageData = zlib.decompress(imageData)
#            print "uncompressed to %s bytes" % len(imageData)
        self.originalcvImage = cv.CreateImage((imageData.width, imageData.height), cv.IPL_DEPTH_8U, 3)
        cv.SetData(self.originalcvImage, imageData.data, imageData.width*3)

        cvImage = self.originalcvImage

        # save stream of images
        if self.saveStream:
            print "saving img ..."
            imagePath = self.fumanoidsPath + "/images/"
            if os.path.isdir(imagePath) == False:
              imagePath = "./"
            cv.SaveImage(imagePath + str(time.time())+".png", cvImage)

        # convert from BGR to RGB
        #cv.CvtColor(cvImage, cvImage, cv.CV_BGR2RGB)

        # create pixbuf
        pixbuf = gtk.gdk.pixbuf_new_from_data(cvImage.tostring(), gtk.gdk.COLORSPACE_RGB, False, 8, imageData.width, imageData.height, imageData.width*3)

        gtk.gdk.threads_enter()
        try:
            self.cameraImage.set_from_pixbuf(pixbuf)
            self.gui.currentImageLbl.set_text("Image from %s" % updateTimestamp)
        finally:
            gtk.gdk.threads_leave()

        self.gui.updateActivity("Image successfully received.")


####################################################################################################

class CameraWindow():

    def __init__(self, mainWindow, gladefile, robotIP):
        try:
            # check whether OpenCV 2.0 interface is installed
            cv.CreateMat(5, 5, cv.CV_32FC1)

        except:
            msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                                    "Please install python-opencv 2.0. On Ubuntu 10.10 / Maverick Meerkat install it as aptitude install python-opencv. On Ubuntu 10.04, please check the instructions in opencv.txt")
            msg.run()
            msg.destroy()


        self.mainWindow    = mainWindow
        self.robotIP       = robotIP
        self.gladefile     = gladefile
        self.wTree         = gtk.glade.XML(self.gladefile, "cameraWindow")
        self.camWindow     = self.wTree.get_widget("cameraWindow")
        self.cameraImage   = self.wTree.get_widget("cameraImage")
        self.lastImageTime = 0
        self.event         = Event()
        self.isActive      = True

        self.activityLabel   = self.wTree.get_widget("activityLbl")
        self.robotNameLbl    = self.wTree.get_widget("robotNameLbl")
        self.currentImageLbl = self.wTree.get_widget("currentImageLbl")

        self.robotNameLbl.set_text(robotIP)

        self.delaySpin = self.wTree.get_widget("updateDelaySpin")
        self.delaySpin.set_value(2)

        self.overlayCbx = self.wTree.get_widget("overlayCbx")
        setComboboxValues(self.overlayCbx, ["Camera image", "Undistorted image", "Projected image", "Color image", "Gradient image" ])
        self.overlayCbx.set_active(0)

        self.scalingCbx = self.wTree.get_widget("scalingCbx")
        setComboboxValues(self.scalingCbx, ["100%", "50%", "25%"])
        self.scalingCbx.set_active(2)

        # connect events
        dic = { "on_stepBtn_clicked"              : self.stepButtonClicked,
                "on_saveBtn_clicked"              : self.saveButtonClicked,
                "on_pauseToggle_toggled"          : self.pauseButtonToggled,
                "on_window_destroy"               : self.destroy,
                "on_cbtnSaveStream_toggled"       : self.toggleSaveStream
              }
        self.wTree.signal_autoconnect(dic)

        # create update thread
        self.imageUpdater = CameraImageUpdater(self, self.event, self.robotIP, self.cameraImage)
        self.imageUpdater.daemon = True
        self.imageUpdater.start()

        # trigger future updates
        gobject.timeout_add(500, self.updateImage)

        self.camWindow.show()

        # update image
        self.updateImage()

    def toggleSaveStream(self, widget):
        self.imageUpdater.saveStream = widget.get_active()

    def destroy(self, widget, data=None):
        self.imageUpdater.running = False
        pass

    def updateActivity(self, text):
        # print "update activity to: %s" % text
        gtk.gdk.threads_enter()
        try:
            self.activityLabel.set_text(text)
        finally:
            gtk.gdk.threads_leave()

    def getUpdateInterval(self):
        return self.delaySpin.get_value()

    def updateImage(self):
        # check whether it is time to update the image
        if self.isActive and time.time() > self.lastImageTime +\
                self.getUpdateInterval():
            self.event.set()
        return True

    def pauseButtonToggled(self, widget):
        self.isActive = not widget.get_active()
        self.wTree.get_widget("stepBtn").set_sensitive(not self.isActive)

    def stepButtonClicked(self, widget):
        self.event.set()

    def saveButtonClicked(self, widget):
        # make a copy of the image
        cvImage = self.imageUpdater.originalcvImage

        # abort if not valid
        if not cvImage:
            return

        dialog = gtk.FileChooserDialog(title="Save image", action=gtk.FILE_CHOOSER_ACTION_SAVE,
                                       buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_SAVE, gtk.RESPONSE_OK))

        response = dialog.run()
        if response == gtk.RESPONSE_OK:
            filename = dialog.get_filename();

        dialog.destroy()

        if filename:
            cv.CvtColor(cvImage, cvImage, cv.CV_RGB2BGR)
            try:
                cv.SaveImage(filename, cvImage)
            except:
                msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                            "Image could not be saved. Did you enter a filename with a supported extension, like .png or .jpg?")
                msg.run()
                msg.destroy()


