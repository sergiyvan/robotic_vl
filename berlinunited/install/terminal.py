####################################################################################################
#
# The Terminals class handles the notebook containing the terminals.
# 
# Based on http://www.mail-archive.com/pygtk@daa.com.au/msg12899.html

try:
    import pygtk
    import gtk
    pygtk.require("2.0")
except:
    pass


try:
    import vte
except:
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                            "Please install VTE (e.g. via 'apt-get install python-vte package').")
    msg.run()
    msg.destroy()
    sys.exit(1)


class Terminals:
    def __init__(self, terminals):
        self.terminals = terminals

    def closeAll(self):
        """Closes all terminals except the WalkerTab."""
        count = self.terminals.get_n_pages()-1
        while (count):
            self.terminals.remove_page( 1 )
            count = count-1
        
        self.addTerminal("Shell", "")

    # Remove a page from the notebook
    def remove_book(self, button, notebook, child):
        page = notebook.page_num(child)
        if page != -1:
            notebook.remove_page(page)
        # Need to refresh the widget -- 
        # This forces the widget to redraw itself.
        notebook.queue_draw_area(0,0,-1,-1)

        if 0 == self.terminals.get_n_pages():
            self.addTerminal("Shell", "")


    def add_icon_to_button(self, button):
        iconBox = gtk.HBox(False, 0)        
        image = gtk.Image()
        image.set_from_stock(gtk.STOCK_CLOSE, gtk.ICON_SIZE_MENU)
        gtk.Button.set_relief(button, gtk.RELIEF_NONE)
        settings = gtk.Widget.get_settings (button);
        (w,h) = gtk.icon_size_lookup_for_settings (settings, gtk.ICON_SIZE_MENU);
        gtk.Widget.set_size_request(button, w + 4, h + 4);
        image.show()
        iconBox.pack_start(image, True, False, 0)
        button.add(iconBox)
        iconBox.show()
        return 

    def create_custom_tab(self, text, notebook, child):
        #create a custom tab for notebook containing a 
        #label and a button with STOCK_ICON
        eventBox = gtk.EventBox()
        tabBox = gtk.HBox(False, 2)
        tabLabel = gtk.Label(text)
        
        tabButton=gtk.Button()
        tabButton.connect('clicked', self.remove_book, notebook, child)
        
        #Add a picture on a button
        self.add_icon_to_button(tabButton)
        iconBox = gtk.HBox(False, 0)
        
        eventBox.show()
        tabButton.show()
        tabLabel.show()
        
        tabBox.pack_start(tabLabel, False)       
        tabBox.pack_start(tabButton, False)
        
        # needed, otherwise even calling show_all on the notebook won't
        # make the hbox contents appear.
        tabBox.show_all()
        eventBox.add(tabBox)
        
        return eventBox
    
    def terminalExit(self, widget):
        self.remove_book(None, self.terminals, widget.parent)
        pass
    
    def button_press_cb(self, widget, event):
        if event.button == 3:
            self.copyToClipboard(widget)
            return True

    def copyToClipboard(self, terminal):
        cb = gtk.clipboard_get(selection = "PRIMARY").wait_for_text()
        gtk.clipboard_get(selection = "CLIPBOARD").set_text(cb)
        return True

    def addTerminal(self, label, command):
        terminal = vte.Terminal()
        terminal.fork_command()
        terminal.set_scrollback_lines(4242)
        terminal.connect("button-press-event", self.button_press_cb)
        terminal.show()

        if len(command) > 0:
            terminal.feed_child(command + '\n')

        scrollbar = gtk.VScrollbar( terminal.get_adjustment() )
        scrollbar.show()

        hbox = gtk.HBox(False, 0)
        hbox.pack_start(terminal, True, True, 0)
        hbox.pack_start(scrollbar, False, True, 0)
        hbox.show()
        
        eventBox = self.create_custom_tab(label, self.terminals, hbox)
        idx = self.terminals.append_page(hbox, eventBox)
        self.terminals.set_current_page(idx)
        terminal.grab_focus()

        terminal.connect("child-exited", self.terminalExit)

    def addWalkerTab(self):
        button = gtk.Button("Hallo Welt!")
        button.show()
       
        hbox = gtk.HBox(False, 0)
        hbox.pack_start(button, True, True, 0)
        hbox.show()
        
        eventBox = self.create_custom_tab("Walker", self.terminals, hbox)
        idx = self.terminals.append_page(hbox, eventBox)
        self.terminals.set_current_page(idx)

