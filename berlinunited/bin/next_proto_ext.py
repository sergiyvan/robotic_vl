#!/usr/bin/env python

"""
Script to show all current proto extensions and find the next free one.
"""

import pprint
import sys
import os
import glob
import re


# each extended msg is saved here with the name of the msg as key
extended_msgs = {}


class ExtendedMsg:
    """
    Infos about a proto msg that gets extended.
    """
    def __init__(self):
        self.slots = []
        self.range_ = []
        self.file_ = None


def main(pbdir):
    """
    iterate over all file
    search for extending messages
    and the extension slots of the extending msgs
    """
    for f in glob.glob(os.path.join(pbdir, "*.proto")):
        with open(f) as fo:
            data = fo.readlines()
            extended_msg = None
            current_msg = None

            for line in data:

                # the name of the current msg
                wanted = re.compile("message *(\w+) ")
                match = wanted.search(line)
                if match:
                    current_msg = match.group(1)

                # find the range for extension slots
                wanted = re.compile("extensions (\d+) to (\w+)")
                match = wanted.search(line)
                if match and current_msg in extended_msgs:
                    extended_msgs[current_msg].range_.append(match.groups())
                    extended_msgs[current_msg].file_ = f

                # search if this is extending another messages
                wanted = re.compile("^extend ([a-zA-Z0-9_.]+) ")
                match = wanted.search(line)
                if match:
                    extended_msg = match.groups()[0].split('.')[-1]

                    # save the name of the extended msg
                    if extended_msg not in extended_msgs:
                        extended_msgs[extended_msg] = ExtendedMsg()

                # save the extensions if the msg is extendig another msg
                if extended_msg and (
                        line.strip().startswith('optional') or
                        line.strip().startswith('repeated') or
                        line.strip().startswith('required')):

                    wanted = re.compile(" *(\w+) *(\w+) *(\w+) *= *(\d+) *;")
                    matches = wanted.search(line)
                    if matches:
                        fieldtype, valuetype_, name, slot = matches.groups()
                        if extended_msg in extended_msgs:
                            formated = int(slot), valuetype_, name, fieldtype
                            extended_msgs[extended_msg].slots.append(formated)
                        else:
                            print "ERROR"

    # print the results
    for key in extended_msgs:
        print '\n', key,  extended_msgs[key].file_
        print 'Extension slots:'
        for min_, max_ in extended_msgs[key].range_:
            print '\tfrom', min_, 'to', max_
        pprint.pprint(sorted(extended_msgs[key].slots))


if __name__ == '__main__':
    usage = "Find free extension slots of proto msgs.\nusage: %s [-h|--help] <protobuf_dir>" % sys.argv[0]

    if len(sys.argv) == 1:
        print usage

    if len(sys.argv) >= 2:
        if sys.argv[1] in ("-h", "--help"):
            print usage
        else:
            pbdir = sys.argv[1]
            main(pbdir)
