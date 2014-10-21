#!/usr/bin/env python2

import os
import sys
import inspect

import SocketServer
from threading import Thread, Event

# set the path of the proto lib because we use a specific version
cwd = os.path.split(inspect.getfile(inspect.currentframe()))[0]
python_protobuf_folder = os.path.abspath( cwd + "/../tools/protobuf/pyshared/" )

if python_protobuf_folder not in sys.path:
    sys.path.insert(0, python_protobuf_folder)


from fu_install import FUmanoidInstall

from optparse import OptionParser
import socket
import threading

try:
    import pygtk
    pygtk.require("2.0")
except:
    import gtk
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR,
            gtk.BUTTONS_OK,
            "Please install PyGTK 2. On Debian/Ubuntu, try 'apt-get \
            install python-gtk2'")
    msg.run()
    msg.destroy()
    pass

try:
    import gtk
    import gtk.glade
except:
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR,
            gtk.BUTTONS_OK,
            "Please install Glade for Python. On Debian/Ubuntu, try \
            'apt-get install python-glade2'")
    msg.run()
    msg.destroy()
    sys.exit(1)


fui = None
server = None

def start_server(host, port):
    server = ThreadedTCPServer((host, port), ThreadedTCPRequestHandler)
    ip, port = server.server_address

    server_thread = threading.Thread(target=server.serve_forever)
    # Exit the server thread when the main thread terminates
    server_thread.setDaemon(True)
    server_thread.start()
    return server

def client(ip, port, message):
    return False # FIXME: remove

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((ip, port))
        sock.send(message)
        sock.settimeout(2)
        response = sock.recv(4)
        sock.close()
    except Exception:
        return False

    return True

####################################################################################################

class ThreadedTCPRequestHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        request = self.request;
        data = request.recv(4)
        fui.showYourself()
        request.send("done")

class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    allow_reuse_address = True
    pass



####################################################################################################

if __name__ == "__main__":
    HOST, PORT = "localhost", 51015

    usage = "Usage: %prog [options] infofile"
    parser = OptionParser(usage=usage)
    parser.add_option("-f", "--fork", dest="fork", help="Do a complete fork", action="store_true", default=False)
    (options, args) = parser.parse_args()

    if len(args) != 1:
        parser.error("Incorrect number of arguments, missing info file for project")

    if False == client(HOST, PORT, 'show'):
        if options.fork == True:
            try:
                # fork and commit suicide
                if os.fork():
                    os._exit(0)

                os.setsid()

                # fork again (2nd child) and exit this fork (1st child), causing
                # the 2nd child to be orphaned (init process is now responsible
                # for it)
                try:
                    if os.fork():
                        os._exit(0)

                except AttributeError:
                    pass
            except AttributeError:
                pass

            # redirect the standard I/O file descriptors to the specified file.
            if (hasattr(os, "devnull")):
                REDIRECT_TO = os.devnull
            else:
                REDIRECT_TO = "/dev/null"

            # this call to open is guaranteed to return the lowest file descriptor,
            # which will be 0 (stdin), since it was closed above.
            os.open(REDIRECT_TO, os.O_RDWR)  # standard input (0)

            # duplicate standard input to standard output and standard error.
            os.dup2(0, 1)                    # standard output (1)
            os.dup2(0, 2)                    # standard error (2)

        # now run it
        try:
            server = start_server(HOST, PORT)
            fui = FUmanoidInstall(server, args[0])
            gtk.gdk.threads_init()
            gtk.gdk.threads_enter()
            gtk.main()
            gtk.gdk.threads_leave()
        except KeyboardInterrupt:
            if fui.messageListener != None:
                fui.messageListener.disconnect()

