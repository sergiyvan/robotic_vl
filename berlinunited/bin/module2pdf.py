#!/usr/bin/env python

"""
File: module2pdf.py
Author: stefan.otte@gmail.com

Description: A simple script to visualize the module structure.

It uses the pydot python package.

If you get an error that the PDF output format is not supported, run

    sudo dot -c

"""


from pydot import Edge
from pydot import Node
from pydot import Graph

import re
import os


COGNITION_SRC = 'src/modules/cognition/'
COGNITION_DEST = 'doc/modules_cognition'

MOTION_SRC = 'src/modules/motion/'
MOTION_DEST = 'doc/modules_motion'

representation_ignore = ['ImageStream', 'Symbols']
modules_ignore = ['Symbols']


class Module:
    """
    Represents a Module in the module conceps of NaoTH/FUmanoids.

    Stupid data object.
    """
    def __init__(self):
        self.name = ''
        self.required = []
        self.provided = []
        self.recycled = []


def get_module_info(text):
    """
    Create Module object for the given text.

    Return the module object.
    """
    module = None
    for row in text:
        # ignore comments
        if row.strip().startswith('//'):
            continue

        # find module
        match = re.search('BEGIN_DECLARE_MODULE\((\w*)\)', row)
        if match:
            token = match.groups()[0]
            ignore = any(elem in token for elem in modules_ignore)
            if not ignore:
                module = Module()
                module.name = match.groups()[0]

        # find required representations
        match = re.search('REQUIRE\((\w*)\)', row)
        if match:
            if not module:
                return
            token = match.groups()[0]
            ignore = any(elem in token for elem in representation_ignore)
            if not ignore:
                module.required.append(token)

        # find provided representations
        match = re.search('PROVIDE\((\w*)\)', row)
        if match:
            if not module:
                return
            token = match.groups()[0]
            ignore = any(elem in token for elem in representation_ignore)
            if not ignore:
                module.provided.append(match.groups()[0])

        # find recycled representations
        match = re.search('RECYCLE\((\w*)\)', row)
        if match:
            if not module:
                return
            token = match.groups()[0]
            ignore = any(elem in token for elem in representation_ignore)
            if not ignore:
                module.recycled.append(match.groups()[0])

        # abort after end of declaration
        match = re.search('END_DECLARE_MODULE', row)
        if match:
            return module


def create_graph(src, destination):
    print "module2pdf.py"
    print 'parsing files...'

    if False == os.path.exists(src):
        return

    modules = []
    all_representations = []
    # travel the path recursively down and create module representations
    for (path, dirs, files) in os.walk(src):
        for file_ in files:
            if file_.endswith('.h') or file_.endswith('.cpp') or file_.endswith('.cxx'):
                # path of the current file
                mod_path = os.path.join(path, file_)

                # create edges
                with open(mod_path) as f:
                    module = get_module_info(f)
                    if module:
                        all_representations.extend(module.required)
                        all_representations.extend(module.provided)
                        all_representations.extend(module.recycled)
                        modules.append(module)

    print "creating graph..."
    graph = Graph()

    # add nodes of representations
    for representation in set(all_representations):
        graph.add_node(Node(representation, style='filled', color='lightgrey',
                fontsize=11))

    # add nodes of modules
    module_names = set([module.name for module in modules])
    for name in module_names:
        graph.add_node(Node(name))

    # add edges
    for module in modules:
        # require -> module
        for representation in module.required:
            graph.add_edge(Edge(representation, module.name))

        # module -> provide
        for representation in module.provided:
            graph.add_edge(Edge(module.name, representation))

        # module -> recycled
        for representation in module.recycled:
            edge = Edge(representation, module.name, color="red", dir="none")
            graph.add_edge(edge)

    # write .dot file
    dot_file = destination + ".dot"
    print 'writing dot file %s...' % dot_file
    g_txt = graph.to_string()
    #print g_txt
    with open(dot_file, 'w') as f:
        f.writelines(g_txt)

    # create pdf
    pdf_file = destination + ".pdf"
    print 'creating pdf file %s...' % pdf_file
    cmd = "dot -Tpdf %s -o %s" % (dot_file, pdf_file)
    os.system(cmd)
    print 'You can open %s now' % pdf_file
    print "DONE"


if __name__ == '__main__':
    create_graph(COGNITION_SRC, COGNITION_DEST)
    create_graph(MOTION_SRC, MOTION_DEST)
