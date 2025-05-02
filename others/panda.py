from direct.showbase.ShowBase import ShowBase
from panda3d.core import WindowProperties
from panda3d.core import AmbientLight
from panda3d.core import Vec4
from panda3d.core import PNMImage
from panda3d.core import DirectionalLight
from direct.gui.OnscreenText import OnscreenText
from panda3d.core import TextNode
from panda3d.core import Vec3
from panda3d.core import Point3
from panda3d.core import Quat
import math

import serial
import threading

def toVec(lst):
    return Vec3(lst[0], lst[1], lst[2])
def toQuat(lst):
    return Quat(lst[0], lst[1], lst[2], lst[3])


class Game(ShowBase):
    def __init__(self):
        ShowBase.__init__(self)

        #self.keyBinds = ['']

        properties = WindowProperties()
        properties.setSize(1920, 1000)
        self.win.requestProperties(properties)
        
        self.model = self.loader.loadModel("trede/dronemodel.glb")
        self.model.reparentTo(self.render)

        self.accArrow = VectorArrow(self)


        #texture = self.loader.loadTexture("trede/ardronetopview.jpg")
        #self.model.setTexture(texture, 1)
        self.model.setColorScale(1, 0.5, 0.5, 1)

        #self.camera.setPos(0, -150, 60)
        self.camera.setPos(0, -100, 30)
        self.camera.setP(-15)
        self.disableMouse()

        mainLight = DirectionalLight("main light")
        self.mainLightNodePath = self.render.attachNewNode(mainLight)
        self.mainLightNodePath.setHpr(45, -45, 0)
        self.render.setLight(self.mainLightNodePath)
        self.render.setShaderAuto()
        
        self.resetVals()
        self.posText = [OnscreenText(text=f'{i[0]}',pos=(i[1], -0.9), scale=0.05, fg=(1, 1, 1, 1), align=TextNode.ALeft) for i in [['pos', -1.8],['vel', -1.2],['acc', -0.6]]]
        self.taskMgr.add(self.frame, "spinCubeTask")
        self.accept("r", self.writeBT, ["a"])
    
    def writeBT(self, text):
        temp = text+"\n"
        ser_out.write(temp.encode())
    

    def resetVals(self):
        self.pos = [Vec3(0,0,0) for i in range(3)]
        self.magnet = Vec3(0,0,0)
        self.angle = Quat(1,0,0,0)
        self.omega = Vec3(0,0,0)
        self.rpms = [0,0,0,0]

    def process(self,data):
        #x, y, z, vx, vy, vx, ax, ay, az, magx, magy, magz, qw, qx, qy, qz, omegax, omegay, omegaz, M1, M2, M3, M4
        points = data.split(',')
        try:
            points = [float(el) for el in points]
        except:
            print("fail"+data)
            print("\n")
            return
        self.pos = [toVec(points[i*3:(i+1)*3]) for i in range(3)]
        self.magnet = toVec(points[9:12])
        self.angle = toQuat(points[12:16])
        self.omega = toVec(points[16:19])
        self.rpms = points[19:23]

    def updateText(self):
        for i, j in enumerate(['pos', 'vel', 'acc']):
            self.posText[i].setText(f"{j}: ({self.pos[i].getX():.2f}, {self.pos[i].getY():.2f}, {self.pos[i].getZ():.2f})")

    def frame(self, task):
        self.updateText()
        self.model.setQuat(self.angle)
        self.accArrow.set(self.pos[2])
        return task.cont

class VectorArrow():
    def __init__(self, game):
        self.model = game.loader.loadModel("trede/arrow.glb")
        self.model.reparentTo(game.render)
    def set(self,vec):
        length = vec.length()
        self.model.setHpr(0,0,0)
        if length==0:
            return
        self.model.setScale(length,2,2)
        yaw = math.atan2(vec.getY(), vec.getX())
        pitch = -math.asin(vec.getZ()/length)
        self.model.setHpr(yaw*180/math.pi,0,pitch*180/math.pi)

game = Game()

#connecta först bluetooth i windows och sedan måste man kolla i device manager -> ports och hitta vilka som dyker upp. 
PORT_OUT = "COM11" 
PORT_IN = "COM12"
BAUD_RATE = 115200

ser_out = serial.Serial(PORT_OUT, BAUD_RATE, timeout=1)
ser_in = serial.Serial(PORT_IN, BAUD_RATE, timeout=1)


ser_out = serial.Serial(PORT_OUT, BAUD_RATE, timeout=1)
ser_in = serial.Serial(PORT_IN, BAUD_RATE, timeout=1)

def read_from_port(ser, label):
    while True:
        if ser.in_waiting:
            data = ser.readline().decode(errors='ignore').strip()
            if data:
                print(f"[{label}] {data}")
                game.process(data)
                
threading.Thread(target=read_from_port, args=(ser_out, "OUT"), daemon=True).start()
threading.Thread(target=read_from_port, args=(ser_in, "IN"), daemon=True).start()



try:
    while True:
        pass
except KeyboardInterrupt:
    ser_out.close()
    ser_in.close()
    print("ejd.")


game.run()
