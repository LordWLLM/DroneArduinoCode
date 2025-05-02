import serial
import time
import threading
ser = serial.Serial(port='COM10', baudrate=115200, timeout=.1) 
import json
import math
from direct.showbase.ShowBase import ShowBase
from panda3d.core import Quat, NodePath
from panda3d.core import Point3


point = [0,0,0,0,0,0,0]
def readSerial():
    global point
    while (True):
        if ser.isOpen():
            data=ser.readline().strip().decode("utf-8")
            if(data):
                try:
                    point=json.loads(data)
                    point[0]*=100
                    point[1]*=100
                    point[2]*=100
                    #print(point)
                except:
                    print("fail"+data)
                    print("\n")
                    continue

class MyApp(ShowBase):
    def __init__(self):
        self.offx=0
        self.offy=0
        self.offz=0
        self.seconds = 10000000
        self.count=60*self.seconds-1
        
        ShowBase.__init__(self)
       
        # Load a cube model
        self.cube = self.loader.loadModel("trede/dronemodel.glb")
        self.cube.reparentTo(self.render)
        self.cube.setPos(0,100,0)
       
        # Position the cube
        min_point, max_point = self.cube.getTightBounds()
        size = max_point - min_point
        print(f"Model dimensions: {size}")

        # Rotate the cube over time
        self.taskMgr.add(self.spin_cube, "spinCubeTask")
   
    def spin_cube(self, task):
        self.count+=1
        """if self.count==60*self.seconds:
            self.count=0
            self.offx = point[0]
            self.offy = point[1]
            self.offz = point[2]"""
        #angle = task.time * 60
        #self.cube.setHpr(angle, angle, angle)
        z = task.time
        #self.cube.setPos(Point3(point[0]-self.offx, point[1]+100-self.offy, point[2]-self.offz))
        self.cube.setQuat(Quat(point[3],point[4],point[5],point[6]))
        print(point)
        return task.cont


threading.Thread(target=readSerial, daemon=True).start()
app = MyApp()
app.run()



