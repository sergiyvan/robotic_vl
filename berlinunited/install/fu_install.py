#from gamepad import Gamepad
from terminal import Terminals
from network import MessageListener, MessageSender
from camera import CameraWindow

import os
import re
import sys
import time
import random
import socket
import struct
import gobject

try:
    import pygtk
    pygtk.require("2.0")
except:
    import gtk
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                            "Please install PyGTK 2. On Debian/Ubuntu, try 'apt-get install python-gtk2'")
    msg.run()
    msg.destroy()
    pass

try:
    import gtk
except:
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                            "Please install Glade for Python. On Debian/Ubuntu, try 'apt-get install python-glade2'")
    msg.run()
    msg.destroy()
    sys.exit(1)

try:
    import vte
except:
    msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK,
                            "Please install VTE (e.g. via 'apt-get install python-vte package').")
    msg.run()
    msg.destroy()
    sys.exit(1)


##########################################################################

####################################################################################################

fui = None
server = None
global robots

####################################################################################################


class FUmanoidInstall():

    def __init__(self, server, projectfile):
        projectfilePath = os.path.dirname( os.path.abspath(projectfile) )
        execfile(projectfile, globals(), locals())

        self.server = server
        self.gladefile = os.path.dirname(os.path.realpath(sys.argv[0])) + "/install.glade"
        self.wTree = gtk.glade.XML(self.gladefile)
        self.window = self.wTree.get_widget("mainWindow")

        self.terminals = Terminals(self.wTree.get_widget("terminals"))
        self.terminals.addTerminal("Shell", "")

        self.tipLabel = self.wTree.get_widget("tipLabel")

        # initialize list of robots
        self.robotView = self.wTree.get_widget("robot")
        self.robotList = gtk.ListStore(str)

        robotColumn = gtk.TreeViewColumn("Robot", gtk.CellRendererText(), text=0)
        robotColumn.set_resizable(False)
        robotColumn.set_sort_column_id(0)
        self.robotView.append_column(robotColumn)
        self.robotView.set_model(self.robotList)
        self.robotView.get_selection().set_mode(gtk.SELECTION_MULTIPLE)

        self.robotLabels = {}
        for robot in self.robots:
            self.robotList.append([robot])

        self.messageListener = None
        self.messageSender = MessageSender()
        self.gamePad = None

        self.window.maximize()

        # connect events
        dic = {
                "on_installButton_clicked"        : self.installButtonClicked,
                "on_newShellButton_clicked"       : self.newShellButtonClicked,
                "on_sshButton_clicked"            : self.sshButtonClicked,
                "on_rebootButton_clicked"         : self.rebootButtonClicked,
                "on_autoOnButton_clicked"         : self.autoOnButtonClicked,
                "on_autoOffButton_clicked"        : self.autoOffButtonClicked,
                "on_putConfigButton_clicked"      : self.putConfigButtonClicked,
                "on_getConfigButton_clicked"      : self.getConfigButtonClicked,
                "on_closeAllButton_clicked"       : self.closeAllButtonClicked,
                "on_quitButton_clicked"           : self.quitButtonClicked,
#                "on_gamepadButton_toggled"        : self.gamepadButtonToggled,
                "on_btnResetMaxSpeeds_clicked"    : self.resetMaxSpeedClicked,
                "on_btnSaveMaxSpeeds_clicked"     : self.setMaxSpeedsClicked,
                "on_cameraButton_clicked"         : self.cameraButtonClicked,
                "on_getImagesButton_clicked"      : self.getImagesButtonClicked,
                "on_window_destroy"               : self.destroy,
              }
        self.wTree.signal_autoconnect(dic)

        gobject.timeout_add(20000, self.updateTipsAndQuotes)

    def updateTipsAndQuotes(self):
        self.tipLabel.set_text( tips[random.randint(0, len(tips)-1)].encode('rot13'))
        return True

    def destroy(self, widget, data=None):
        if (self.messageListener != None):
            self.messageListener.disconnect()
        gtk.main_quit()

    def showYourself(self):
        self.window.present()
        pass

    def modalSSH(self, cmd):
        wTreeDlg = gtk.glade.XML(self.gladefile, "sshDialog")
        self.sshDlg = wTreeDlg.get_widget("sshDialog")
        sshDlgH = wTreeDlg.get_widget("sshDialogH")

        terminal = vte.Terminal()
        terminal.connect("child-exited", lambda x: self.sshDlg.destroy())
        terminal.fork_command()
        terminal.show()

        sshDlgH.add(terminal)
        terminal.feed_child(cmd + '\n')

        self.sshDlg.run()
        self.sshDlg.destroy()

    def newStateLabel(self, text, vbox):
        label = gtk.Label()
        label.set_text(text)
        label.show()

        vbox.pack_start(label, True, False)
        return label

    def getText(self, prompt, label):
        dialog = gtk.MessageDialog(None, gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                   gtk.MESSAGE_QUESTION, gtk.BUTTONS_OK_CANCEL, None)
        dialog.set_markup(prompt)

        entry = gtk.Entry()
        entry.connect("activate", lambda x: dialog.response(gtk.RESPONSE_OK))

        hbox = gtk.HBox()
        hbox.pack_start(gtk.Label(label), False, 5, 5)
        hbox.pack_end(entry)

        dialog.vbox.pack_end(hbox, True, True, 0)
        dialog.show_all()

        dialog.run()
        text = entry.get_text()
        dialog.destroy()
        return text

    def installButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow,
                (self.install, None))

    def newShellButtonClicked(self, widget):
        self.terminals.addTerminal("Shell", '')

    def closeAllButtonClicked(self, widget):
        self.terminals.closeAll()

    def quitButtonClicked(self, widget):
        self.destroy(widget)

    def sshButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow, (self.ssh, None))

    def rebootButtonClicked(self, widget):
        """User clicked on the reboot button, check that this was intentional
        and if yes, reboot"""
        if False == self.ensureSelection():
            return

        msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_QUESTION, gtk.BUTTONS_YES_NO,
                                "Do you really want to reboot the selected robot(s)?")
        res = msg.run()
        msg.destroy()

        if (res == gtk.RESPONSE_YES):
            self.robotView.get_selection().selected_foreach(self.handleRow, (self.reboot, None))
        return

    def autoOnButtonClicked(self, widget):
        if False == self.ensureSelection():
            return

        msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_QUESTION, gtk.BUTTONS_YES_NO,
                                "Do you want to make the autostart remount the root filesystem read-only? (Allows for a faster boot in case of crash)")
        res = msg.run()
        msg.destroy()

        if (res == gtk.RESPONSE_YES):
            self.robotView.get_selection().selected_foreach(self.handleRow, (self.autoStart, (True, False)))
        elif (res == gtk.RESPONSE_NO):
            self.robotView.get_selection().selected_foreach(self.handleRow, (self.autoStart, (True, True)))

    def autoOffButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow, (self.autoStart, (False, False)))

    def putConfigButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow, (self.uploadConfig, None))

    def getConfigButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow, (self.downloadConfig, None))

    def getImagesButtonClicked(self, widget):
        self.robotView.get_selection().selected_foreach(self.handleRow, (self.downloadImages, None))

    def cameraButtonClicked(self, widget):
        robotIP = self.getIpOfSelectedRobot()
        if robotIP == None:
            msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK, "Please select a robot")
            msg.run()
            msg.destroy()
        else:
            camWindow = CameraWindow(self, self.gladefile, robotIP)


    def resetMaxSpeedClicked(self, widget):
        """Reset the max speed values of the walker and display them in the walker tab"""
        self.gamePad.reset_max_speed_values()
        fw, bw, ro, sw = self.gamePad.get_max_speed_values()
        self.wTree.get_widget("entForward").set_text(str(fw))
        self.wTree.get_widget("entBackward").set_text(str(bw))
        self.wTree.get_widget("entRotation").set_text(str(ro))
        self.wTree.get_widget("entSideward").set_text(str(sw))

    def setMaxSpeedsClicked(self, widget):
        self.gamePad.set_max_speed_values(
                                          self.wTree.get_widget("entForward").get_text(),
                                          self.wTree.get_widget("entBackward").get_text(),
                                          self.wTree.get_widget("entRotation").get_text(),
                                          self.wTree.get_widget("entSideward").get_text(),
                                          )

    def gamepadButtonToggled(self, widget):
        """Activate/deactivate the gamepad which can remote control the robot"""
        walkerTab = self.wTree.get_widget("vboxWalker")
        # activate gamepad
        robotIP = self.getIpOfSelectedRobot()
        if widget.get_active() and self.gamePad == None and not robotIP == None:
            self.gamePad = Gamepad(self.messageSender, robotIP, self.wTree)
            # only proceed if gamepad initialized
            if self.gamePad.j == None:
                self.gamePad = None
                widget.set_active(False)
                return
            self.gamePad.start()
            widget.set_label("Deactivate Gamepad Control")
            walkerTab.show()
            self.terminals.terminals.set_current_page(0)
            walkerTab.grab_focus()
        # deactivate gamepad
        elif not widget.get_active() and not self.gamePad == None:
            self.gamePad.quit()
            del self.gamePad
            self.gamePad = None
            widget.set_label("Activate Gamepad Control")
            walkerTab.hide()
        else:
            widget.set_active(False)

    def ensureSelection(self):
        """Checks whether one or more robots are selected

        return true if one or more robots are selected else false"""
        if 0 == self.robotView.get_selection().count_selected_rows():
            msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_ERROR, gtk.BUTTONS_OK, "Please select a robot first!")
            msg.run()
            msg.destroy()
            return False
        else:
            return True

    def extractRobotData(self, robotString):
        m = re.search('^(.*) \((.*)\)( #(.*))?', robotString)
        if (m.group(4) == None):
            robotID = None
        else:
            robotID = int(m.group(4))
        return m.group(1), m.group(2), robotID

    def getIpOfSelectedRobot(self):
        """Return the IP of the selected robot"""
        countOfSelectedRobot = self.robotView.get_selection().count_selected_rows()
        if countOfSelectedRobot == 1:
            (model, pathlist) = self.robotView.get_selection().get_selected_rows()
            for i in pathlist:
                (robotName, robotIP, robotID) = self.extractRobotData(model[i[0]][0])
                return robotIP
        return None

    def handleRow(self, model, path, iter, params):
        robotName, robotIP, robotID = self.extractRobotData(self.robotList.get_value(iter, 0))
        params[0](robotName, robotIP, params[1])

    def ssh(self, name, ip, params):
        self.terminals.addTerminal(name, "ssh -Y -C root@" + ip)

    def install(self, name, ip, param):
        """Install binaries on robot with IP ip"""
        cmd = "rsync -z --progress " + self.filesToCopy + " root@" + ip + ": \
                || (echo; echo 'Error on install'; read enter); exit"
        self.modalSSH(cmd)

    def reboot(self, name, ip, params):
        self.terminals.addTerminal("Reboot " + name, "ssh -C root@" + ip + " /sbin/reboot; exit");

    def autoStart(self, name, ip, params):
        active, rw = params[0], params[1]
        cmd = "ssh root@" + ip

        if True == active:
            autostartFile = "autostart"
            if True == rw:
                autostartFile += "-rw"

            cmd += " \"( "
            cmd +=    "rm -f autostart autostart-rw; ";
            cmd +=    "touch " + autostartFile + "; true";
            cmd += " )\" && exit; echo ERROR, please try again.";
        else:
            cmd += " \"( ";
            cmd +=    "mount -o remount,rw / && ";
            cmd +=    "rm -f autostart autostart-rw && true";
            cmd += " )\" && exit; echo ERROR, please try again.";
            pass

        self.modalSSH(cmd)

    def uploadConfig(self, name, ip, params):
        if name == "cable":
            name = self.getText("Please enter the (correct) name of the robot (refer to the list of names in install.py).", "Name:")
            if name == "":
                return;

        msg = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_QUESTION, gtk.BUTTONS_YES_NO,
                                "Do you really want to overwrite the configuration on " + name + " with your local files?")
        res = msg.run()
        msg.destroy()

        if (res == gtk.RESPONSE_YES):
            cmd = ""
            for configFile in str.split(self.configFiles):
                localFile = self.configPath + "/" + configFile + "-" + name
                cmd += "scp -C \"" + localFile + "\" root@" + ip + ":config/" + configFile + "; "
            self.modalSSH(cmd)

    def downloadConfig(self, name, ip, params):
        if name == "cable":
            name = self.getText("Please enter the (correct) name of the robot (refer to the list of names in install.py).", "Name:")
            if name == "":
                return;

        cmd = ""
        for configFile in str.split(self.configFiles):
            localFile = self.configPath + "/" + configFile + "-" + name
            cmd += "scp -C root@" + ip + ":config/" + configFile + " " + localFile + "; "
        self.modalSSH(cmd)

    def downloadImages(self, name, ip, params):
        cmd  = "scp -C root@" + ip + ":image-*.pbi \"" + self.imagePath + "/\" && ssh root@" + ip + " rm -f image-*.pbi"
        self.modalSSH(cmd)


####################################################################################################

# tip texts
tips = [ "Vg'f abg n oht, vg'f n srngher."
       , "Erny cebtenzzref qba'g pbzzrag gurve pbqr - vg jnf uneq gb jevgr, vg fubhyq or uneq gb haqrefgnaq."
       , "Gur gebhoyr jvgu cebtenzzref vf gung lbh pna arire gryy jung n cebtenzzre vf qbvat hagvy vg'f gbb yngr. - Frlzbhe Penl"
       , "Qba'g jbeel vs vg qbrfa'g jbex evtug. Vs rirelguvat qvq, lbh'q or bhg bs n wbo. - Zbfure'f Ynj bs Fbsgjner Ratvarrevat"
       , "Gur orfg guvat nobhg n obbyrna vf rira vs lbh ner jebat, lbh ner bayl bss ol n ovg."
       , "P++: Jurer sevraqf unir npprff gb lbhe cevingr zrzoref."
       , "Gur svefg 90% bs gur pbqr nppbhagf sbe gur svefg 90% bs gur qrirybczrag gvzr.  Gur erznvavat 10% bs gur pbqr nppbhagf sbe gur bgure 90% bs gur qrirybczrag gvzr. - Gbz Pnetvyy"
       , "Nal pbqr bs lbhe bja gung lbh unira'g ybbxrq ng sbe fvk be zber zbaguf zvtug nf jryy unir orra jevggra ol fbzrbar ryfr. - Rntyrfba'f Ynj"
       , "Vs qrohttvat vf gur cebprff bs erzbivat ohtf, gura cebtenzzvat zhfg or gur cebprff bs chggvat gurz va. - Rqftre J. Qvwxfgen"
       , "Gurer ner gjb jnlf gb jevgr reebe-serr cebtenzf; bayl gur guveq bar jbexf. - Nyna W. Creyvf"
       , "Nyjnlf pbqr nf vs gur thl jub raqf hc znvagnvavat lbhe pbqr jvyy or n ivbyrag cflpubcngu jub xabjf jurer lbh yvir. - Znegva Tbyqvat"
       , "Gb ree vf uhzna, ohg gb ernyyl sbhy guvatf hc lbh arrq n pbzchgre."
       , "N ebobg znl abg vawher n uhzna orvat, be, guebhtu vanpgvba, nyybj n uhzna orvat gb pbzr gb unez. [Gur Svefg Ynj bs Ebobgvpf] - Vfnnp Nfvzbi"
       , "Gurer ner bayl 10 glcrf bs crbcyr va gur jbeyq: Gubfr jub haqrefgnaq ovanel, naq gubfr jub qba'g."
       , "Nqqvat znacbjre gb n yngr fbsgjner cebwrpg znxrf vg yngre - S. Oebbxf, Gur Zlguvpny Zna-Zbagu"
       , "Nal fhssvpvragyl nqinaprq oht vf vaqvfgvathvfunoyr sebz n srngher. - Oehpr Oebja"
       , "Bar zna'f pbafgnag vf nabgure zna'f inevnoyr. - Nyna W. Creyvf"
       , "Vs lbh unir n cebprqher jvgu 10 cnenzrgref, lbh cebonoyl zvffrq fbzr. - Nyna W. Creyvf"
       , "Vg vf rnfvre gb punatr gur fcrpvsvpngvba gb svg gur cebtenz guna ivpr irefn. - Nyna W. Creyvf"
       , "Va gurbel, gurbel naq cenpgvpr ner gur fnzr. Va cenpgvpr, gurl nera'g rira pybfr"
       , "V ybir qrnqyvarf. V yvxr gur jubbfuvat fbhaq gurl znxr nf gurl syl ol. - Qbhtynf Nqnzf"
       , "P cebtenzzref arire qvr. Gurl ner whfg pnfg vagb ibvq."
       , "Cebtenzzvat vf na neg sbez gung svtugf onpx"
       , "Cebtenzzre - na betnavfz gung gheaf pbssrr vagb fbsgjner."
       , "N cebtenzzre vf fbzrbar jub fbyirf n ceboyrz lbh qvqa'g xabj lbh unq va n jnl lbh qba'g haqrefgnaq."
       , "Pbzchgre cebtenzzref qb vg olgr ol olgr"
       , "Vs ohvyqref ohvyg ohvyqvatf gur jnl cebtenzzref jebgr cebtenzf, gur svefg jbbqcrpxre gb pbzr nybat jbhyq qrfgebl pvivyvmngvba"
       , "Orjner bs pbzchgre cebtenzzref gung pneel fperjqeviref."
       , "Erny Cebtenzzref qba'g jevgr fcrpf - hfref fubhyq pbafvqre gurzfryirf yhpxl gb trg nal cebtenzf ng nyy naq gnxr jung gurl trg."
       , "Erny Cebtenzzref hfr P fvapr vg'f gur rnfvrfg ynathntr gb fcryy."
       , "Nyy cebtenzzref ner cynljevtugf naq nyy pbzchgref ner ybhfl npgbef."
       , "N pbzchgre vf n fghcvq znpuvar jvgu gur novyvgl gb qb vaperqvoyl fzneg guvatf, juvyr cebtenzzref ner fzneg crbcyr jvgu gur novyvgl gb qb vaperqvoyl fghcvq guvatf. N cresrpg zngpu."
       , "Fbzrgvzrf vg cnlf gb fgnl va orq va Zbaqnl, engure guna fcraqvat gur erfg bs gur jrrx qrohttvat Zbaqnl'f pbqr."
       , "N tbbq cebtenzzre vf fbzrbar jub nyjnlf ybbxf obgu jnlf orsber pebffvat n bar-jnl fgerrg."
       , "Sebz n cebtenzzre'f cbvag bs ivrj, gur hfre vf n crevcureny gung glcrf jura lbh vffhr n ernq erdhrfg."
       , "V ernyyl ungr guvf qnea znpuvar;\nV jvfu gung gurl jbhyq fryy vg.\nVg jba'g qb jung V jnag vg gb,\nohg bayl jung V gryy vg."
       , "Bar zna'f penccl fbsgjner vf nabgure zna'f shyy gvzr wbo."
       , "Jr fubhyq sbetrg nobhg fznyy rssvpvrapvrf, fnl nobhg 97% bs gur gvzr:  cerzngher bcgvzvmngvba vf gur ebbg bs nyy rivy. - P.N.E. Ubner, dhbgrq ol Qbanyq Xahgu"
       , "Cebtenzzvat vf n enpr orgjrra fbsgjner ratvarref fgevivat gb ohvyq ovttre naq orggre vqvbg-cebbs cebtenzf, naq gur havirefr gelvat gb cebqhpr ovttre naq orggre vqvbgf. Fb sne, gur havirefr vf jvaavat."
       , "Ernql, sver, nvz:  gur snfg nccebnpu gb fbsgjner qrirybczrag.  Ernql, nvz, nvz, nvz, nvz:  gur fybj nccebnpu gb fbsgjner qrirybczrag."
       , "Fubhyq neenl vaqvprf fgneg ng 0 be 1?  Zl pbzcebzvfr bs 0.5 jnf erwrpgrq jvgubhg, V gubhtug, cebcre pbafvqrengvba. - Fgna Xryyl-Obbgyr"
       , "Jura n cebtenzzvat ynathntr vf perngrq gung nyybjf cebtenzzref gb cebtenz va fvzcyr Ratyvfu, vg jvyy or qvfpbirerq gung cebtenzzref pnaabg fcrnx Ratyvfu."
       , "99 yvggyr ohtf va gur pbqr,\n99 ohtf va gur pbqr,\nSvk bar oht, pbzcvyr vg ntnva,\n100 yvggyr ohtf va gur pbqr.\n(tb gb fgneg vs ohtf>0)"
       , "Cebsnavgl vf gur bar ynathntr nyy cebtenzzref xabj orfg."
       , "Vg'f 5.50 n.z.... Qb lbh xabj jurer lbhe fgnpx cbvagre vf ?"
       , "P /a./: N cebtenzzvat ynathntr gung vf fbeg bs yvxr Cnfpny rkprcg zber yvxr nffrzoyl rkprcg gung vg vfa'g irel zhpu yvxr rvgure bar, be nalguvat ryfr. Vg vf rvgure gur orfg ynathntr ninvynoyr gb gur neg gbqnl, be vg vfa'g."
       , "Vs vg jnfa'g sbe P, jr'q or jevgvat cebtenzf va ONFV, CNFNY, naq BOBY."
       , "V unir lrg gb zrrg n P pbzcvyre gung vf zber sevraqyl naq rnfvre gb hfr guna rngvat fbhc jvgu n xavsr."
       , "#qrsvar DHRFGVBA ((oo) || !(oo)) - Funxrfcrner"
       , "... bar bs gur znva pnhfrf bs gur snyy bs gur Ebzna Rzcver jnf gung, ynpxvat mreb, gurl unq ab jnl gb vaqvpngr fhpprffshy grezvangvba bs gurve P cebtenzf"
       , "Havk vf hfre-sevraqyl. Vg'f whfg irel fryrpgvir nobhg jub vgf sevraqf ner."
       , "Gurl qba'g znxr ohtf yvxr Ohaal nalzber."
       , "'Nyjnlf nccyl gur yngrfg hcqngrf' naq 'Vs vg nva'g oebxr, qba'g svk vg' ner gur gjb ehyrf bs flfgrz nqzvavfgengvba..."
       , "Ba gjb bppnfvbaf V unir orra nfxrq [ol zrzoref bs Cneyvnzrag]: 'Cenl, Ze. Onoontr, vs lbh chg vagb gur znpuvar jebat svtherf, jvyy gur evtug nafjref pbzr bhg ?' V nz abg noyr evtugyl gb ncceruraq gur xvaq bs pbashfvba bs vqrnf gung pbhyq cebibxr fhpu n dhrfgvba. - Puneyrf Onoontr."
       , "Pbzchgref znxr irel snfg, irel npphengr zvfgnxrf."
       , "Vs vg jbexf, yrnir vg nybar - gurer'f ab arrq gb haqrefgnaq vg. Vs vg snvyf, gel gb svk vg - gurer'f ab gvzr gb haqrefgnaq vg."
       , "Jbhyq lbh engure Grfg-Svefg, be Qroht-Yngre ?"
       , "Yvsr jbhyq or fb zhpu rnfvre vs jr bayl unq gur fbhepr pbqr"
       , "Orjner bs ohtf va gur nobir pbqr; V unir bayl cebirq vg pbeerpg, abg gevrq vg. - Qbanyq R. Xahgu."
       , "Jura va qbhog, hfr oehgr sbepr.\n\nVs oehgr sbepr qbrfa'g fbyir lbhe ceboyrzf, gura lbh nera'g hfvat rabhtu. - Xra Gubzcfba."
       , "Vs vg'f abg ba sver, vg'f n fbsgjner ceboyrz."
       , "Zl fbsgjner arire unf ohtf. Vg whfg qrirybcf enaqbz srngherf."
       , "Gur bayl qvssrerapr orgjrra n oht naq n srngher vf gur qbphzragngvba"
       , "Gb haqrefgnaq erphefvba, lbh svefg arrq gb haqrefgnaq erphefvba"
       , "Guvf fgngrzrag vf snyfr."
       , "Gur zbfg rkpvgvat cuenfr gb urne va fpvrapr, gur bar gung urenyqf arj qvfpbirevrf, vf abg 'Rherxn!' ohg 'Gung'f shaal...' - Vfnnp Nfvzbi"
       , "Qrohttref qba'g erzbir ohtf. Gurl bayl fubj gurz va fybj zbgvba."
       , "Zl qrsvavgvba bs na rkcreg va nal svryq vf n crefba jub xabjf rabhtu nobhg jung'f ernyyl tbvat ba gb or fpnerq."
       , "Jr orggre uheel hc naq fgneg pbqvat, gurer ner tbvat gb or n ybg bs ohtf gb svk"
       , "Gb vgrengr vf uhzna, gb erphefr qvivar."
       , "Rirel ynathntr unf na bcgvzvmngvba bcrengbe. Va P++ gung bcrengbe vf '//'"
       , "Vs jr'er fhccbfrq gb jbex va Urk, jul unir jr bayl tbg N svatref?"
       , "Jrrxf bs pbqvat pna fnir lbh ubhef bs cynaavat."
       , "Gur trarengvba bs enaqbz ahzoref vf gbb vzcbegnag gb or yrsg gb punapr."
       , "Nalbar jub pbafvqref nevguzrgvp zrgubqf bs cebqhpvat enaqbz qvtvgf vf, bs pbhefr, va n fgngr bs fva. - Wbua iba Arhznaa (1951)"
       , "Tbq pbhyq perngr gur jbeyq va fvk qnlf orpnhfr ur qvqa'g unir gb znxr vg pbzcngvoyr jvgu gur cerivbhf irefvba"
       , "Vs Wnin unq gehr tneontr pbyyrpgvba, zbfg cebtenzf jbhyq qryrgr gurzfryirf hcba rkrphgvba."
       , "Ubj qbrf n ynetr fbsgjner cebwrpg trg gb or bar lrne yngr? Nafjre: Bar qnl ng n gvzr! - Serq Oebbxf (Gur Zlguvpny Zna-Zbagu)"
       , "Vs V unq zber gvzr, V jbhyq unir jevggra n fubegre yrggre. - Pvpreb"
       , "Gurbel vf jura lbh xabj fbzrguvat, ohg vg qbrfa'g jbex. Cenpgvpr vf jura fbzrguvat jbexf, ohg lbh qba'g xabj jul. Cebtenzzref pbzovar gurbel naq cenpgvpr: Abguvat jbexf naq gurl qba'g xabj jul."
       , "Jura V nz jbexvat ba n ceboyrz V arire guvax nobhg ornhgl. V guvax bayl ubj gb fbyir gur ceboyrz. Ohg jura V unir svavfurq, vs gur fbyhgvba vf abg ornhgvshy, V xabj vg vf jebat. E. Ohpxzvafgre Shyyre"
       , "Zna vf gur orfg pbzchgre jr pna chg nobneq n fcnprpensg...naq gur bayl bar gung pna or znff cebqhprq jvgu hafxvyyrq ynobe. - Jreaure iba Oenha"
       , "V jbhyq ybir gb punatr gur jbeyq, ohg gurl jba'g tvir zr gur fbhepr pbqr"
       , "Guvax gjvpr orsber lbh fgneg cebtenzzvat be lbh jvyy cebtenz gjvpr orsber lbh fgneg guvaxvat."
       , "Fvzcyvpvgl vf cererdhvfvgr sbe eryvnovyvgl. -Rqftre Qvwxfgen"
       , "Pbasvqrapr, a.: Gur srryvat lbh unir orsber lbh haqrefgnaq gur fvghngvba"
       , "Vs gur pbqr naq gur pbzzragf qvfnterr, gura obgu ner cebonoyl jebat"
       , "Va P, vgf rnfl gb fubbg lbhefrys va gur sbbg. P++ znxrf vg zber qvssvphyg, ohg jura lbh qb, lbh'yy oybj lbhe jubyr yrt bss."
       , "Crbcyr jub qrny jvgu ovgf fubhyq rkcrpg gb trg ovggra"
       , "Nyjnlf pbqr nf vs n fvatyr oht jvyy oevat gur ohvyqvat qbja."
       , "Abguvat vf zber creznarag guna n grzcbenel fbyhgvba"
       , "vs (!xvyy) fgeratgu++;"
       , "Vg'f uneq gb ernq guebhtu n obbx ba gur cevapvcyrf bs zntvp jvgubhg tynapvat ng gur pbire crevbqvpnyyl gb znxr fher vg vfa'g n obbx ba fbsgjner qrfvta."
       , "Lbh pna'g ryvzvangr ceboyrzf, ohg lbh pna znxr genqrf gb trg ceboyrzf gung lbh cersre bire gur barf lbh unir abj. "
       , "Lbhe pbqr vf obgu tbbq naq bevtvany. Hasbeghangryl gur cnegf gung ner tbbq ner abg bevtvany, naq gur cnegf gung ner bevtvany ner abg tbbq."
       , "Gur fbbare lbh trg oruvaq va lbhe jbex, gur zber gvzr lbh unir gb pngpu hc."
       , "Jura lbh urne ubbs orngf, guvax ubefrf, abg mroenf."
       , "Vs jr pna'g svk vg, gura vg nva'g oebxr."
       , "Fbsgjner naq pngurqenyf ner zhpu gur fnzr - svefg jr ohvyq gurz, gura jr cenl"
       , "Nyy cebtenzzref ner bcgvzvfgf - Serqrevpx Oebbxf"
       , "Sbe rirel pbzcyrk ceboyrz gurer vf na nafjre gung vf pyrne, fvzcyr, naq jebat."
       ]

