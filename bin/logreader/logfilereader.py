import struct
from protobuf import crf_modulelog_pb2


class LogFileReader:
    """
    Read proto log files (*.pbl), create an index (of frames) and allow to
    jump around within the proto file to retrieve data as a crf_modulelog_pb2.
    """

    def __init__(self, fname):
        self.f = open(fname)
        # index is a list of (startpos of frame, size of frame)
        self.index = self.create_index()
        self.currentpos = -1

    def create_index(self):
        """
        Index of the file.
        """
        result = []
        try:
            while True:
                frame_number, = struct.unpack(">I", self.f.read(4))
                log_size, = struct.unpack(">I", self.f.read(4))
                result.append((self.f.tell(), log_size))
                self.f.seek(log_size, 1)
        except Exception:
            pass
        finally:
            self.f.seek(0)
            return result

    def _as_proto(self, pos):
        """
        Convert the data to a proto moduleframe.
        """
        print '=' * 78
        print 'frame number:', self.currentpos

        start, end = self.index[self.currentpos]
        self.f.seek(start)
        data = self.f.read(end)

        msg = crf_modulelog_pb2.ModuleFrame()
        msg.ParseFromString(data)

        return msg

    def get_next_frame(self):
        """
        Read the next frame
        """
        if self.currentpos >= len(self.index) - 1:
            print "Reached EOF"
        else:
            self.currentpos += 1
        return self._as_proto(self.currentpos)

    def get_previous_frame(self):
        """
        Read the previous frame
        """
        if self.currentpos <= 0:
            print "Reached beginning of file"
        else:
            self.currentpos -= 1
        return self._as_proto(self.currentpos)
