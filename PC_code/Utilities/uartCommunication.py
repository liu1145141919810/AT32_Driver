import time 
import serial
import select
import sys
from . import publicTool as tl
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
        self.ser.write(
            (tx_msg + "\n").encode()#patching the line change
            )
        
    def run(self):
        while True:
            ready,_,_=select.select(
                            [sys.stdin,self.ser],[],[])
            for source in ready:
                if source == sys.stdin:
                    tx_msg=sys.stdin.readline().rstrip()
                    self.typeinOrder(tx_msg)
                elif source == self.ser:
                    self.resManager.receive(self.ser)
            time.sleep(tl.CHECKING_DELAY)