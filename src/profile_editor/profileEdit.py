from copy import deepcopy
import shutil
import sys
import os
from os.path import expanduser, join, isfile, exists
from platform import platform, system
from PyQt5 import QtWidgets, QtCore, QtGui
from profileEditUI import Ui_MainWindow

class Gui(QtWidgets.QMainWindow):
    def __init__(self, parent=None):
        QtWidgets.QWidget.__init__(self, parent)
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.ui.capsDropSlider.valueChanged.connect(self.test1)
        self.ui.capsDropSlider.sliderPressed.connect(self.test1)
        self.ui.programListWidget.clicked.connect(self.profileSelect)

        self.home = expanduser('~')
        self.localConf = join(self.home, '.config/firejail')
        if exists(self.localConf) is False:
            os.mkdir(self.localConf)

        self.mainConf = '/etc/firejail'
        self.profilePath = ''

        self.getProfiles()

    def getProfiles(self):
        '''Find all profiles.'''
        # Get all potential profiles
        localFiles = os.listdir(self.localConf)
        mainFiles= os.listdir(self.mainConf)

        # Combine the list of potential profiles from ~/.config/firejail/
        # and /etc/firejail/
        allFiles = deepcopy(mainFiles)
        allFiles.extend(x for x in localFiles if x not in allFiles)
        allFiles = sorted(allFiles)

        # Now get a list of all files that are profiles
        profiles = []
        for filename in allFiles:
            if filename.endswith('profile'):
                profiles.append(filename)
                self.ui.programListWidget.addItem(filename)

    def test1(self):
        print(self.localConf)
        # self.ui.capsDropSlider.setValue(1)
        # print(self.ui.capsDropSlider.value())

    def profileSelect(self):
        '''Select the target profile from the list and figure out its path.'''
        item = self.ui.programListWidget.currentItem().text()
        if isfile(join(self.localConf, item)):
            self.profilePath = join(self.localConf, item)
        else:
            self.profilePath = join(self.mainConf, item)
            msg = ('<p><b>This profile is not currently in your local Firejail configuration folder.</b>'
                '<br>Would you like to copy it there now for editing?</br></p>')
            toCopy = QtWidgets.QMessageBox.question(self, 'Editor', msg)
            if toCopy == QtWidgets.QMessageBox.Yes:
                shutil.copy(self.profilePath, self.localConf)
                self.profilePath = join(self.localConf, item)
            else:
                return 0

        # Now read the current profile's
        self.readProfile()

    def readProfile(self):
        '''Read the user-selected profile from ~/.config/firejail/'''
        with open(self.profilePath, 'r+') as conf:
            self.oldConfig = conf.read().splitlines()

        self.newConfig = deepcopy(self.oldConfig)
        print(self.newConfig)


def main():
    app = QtWidgets.QApplication.instance()

    if not app:
        app = QtWidgets.QApplication(sys.argv)

    ex1 = Gui()
    ex1.show()
    ex1.activateWindow()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
