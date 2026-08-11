import time 
import serial
import select
import sys
from .. import publicTool as tl
#Both Usart Input and Output are administrated here
class UartCommunication:
    def __init__(self,port, baudrate, resManager, cmdManager):
        self.resManager = resManager
        self.cmdManager = cmdManager
        try:
            self.ser = serial.Serial(
                            port=port,
                            baudrate=baudrate,
                            timeout=1)
        except serial.SerialException as e:
            raise Exception("Failed to open serial port: {}".format(e))
        tl.demoPrint("Connection established.")
        tl.demoPrint("Now waiting for the device to start...")
        while True:
            if self.ser.in_waiting:
                tl.demoPrint("Waiting Stablity...")
                self.resManager.receive(self.ser)
                state, length, payload = self.resManager.outQueue()
                if payload!=None and payload.decode(errors="ignore") == "AT32_READY\r\n":
                    tl.demoPrint("Device start working successfully!")
                    break
                else:
                    tl.demoPrint("Startup Failed, please check the device.")
            start_time = time.time()
            if time.time() - start_time > tl.WAITING_START:
                tl.demoPrint("Timeout: Device did not start working within the waiting time.")
                #raise Exception("Device did not start working within the waiting time.")
            time.sleep(tl.CHECKING_DELAY)
    # Anaysis the order type into the host device
    def typeinOrder(self,tx_msg):
        if(tx_msg == "HostExit"):
            tl.demoPrint("Host already exit the communication.")
            self.cmdManager.setExitFlag()
            sys.exit(0)
        frame=self.resManager.frameup(tl.PUBLIC_DEFAULT_STATE,tx_msg)+b'\r\n'
        self.ser.write(frame)
    def calibration(self):
        #Time calibration
        t=time.time()
        t_str=time.strftime("%y %m %d %w %H %M %S", time.localtime(t))
        tl.demoPrint("Time "+t_str)
        tl.demoPrint("Time")
        #Calibration emit
        frame=self.resManager.frameup(tl.PUBLIC_DEFAULT_STATE,"Calibrate "+t_str)+b'\r\n'
        self.ser.write(frame)
        frame=self.resManager.frameup(tl.PUBLIC_DEFAULT_STATE,"Calibrate")+b'\r\n'
        self.ser.write(frame)
    # ==============Out Interface Area===============
    def prework(self):
        self.calibration();
    def tell_transmit(self):
        ready,_,_=select.select([sys.stdin,self.ser],[],[])
        if sys.stdin in ready:
            return True
        return False
    def tell_receive(self):
        ready,_,_=select.select([sys.stdin,self.ser],[],[])
        if self.ser in ready:
            return True
        return False
    def transmit(self):
        tx_msg=sys.stdin.readline().rstrip()
        self.typeinOrder(tx_msg)
    def receive(self):
        self.resManager.receive(self.ser)