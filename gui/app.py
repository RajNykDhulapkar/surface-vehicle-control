from PyQt5 import QtWidgets, QtGui
from PyQt5.QtWebEngineWidgets import QWebEngineView
import sys
import os

baseDir = os.path.dirname(__file__)

try:
    from ctypes import windll  # Only exists on Windows.
    myappid = 'com.app.surfaceVehicleControl.1.0.0'
    windll.shell32.SetCurrentProcessExplicitAppUserModelID(myappid)
except ImportError:
    pass


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        self.initUI()

    def initUI(self):

        self.setGeometry(300, 200, 1400, 800)
        self.setWindowTitle("Surface Vehicle Control")
        self.setWindowIcon(QtGui.QIcon(
            os.path.join(baseDir, 'assets/icon.ico')))

        self.webEngineView = QWebEngineView(self)
        self.loadMap()

        self.show()

    def loadMap(self):
        with open("static/map.html", "r") as file:
            html = file.read()
            self.webEngineView.setHtml(html)


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    w = MainWindow()
    app.exec()
