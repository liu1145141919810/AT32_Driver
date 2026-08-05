import queue
from . import publicTool as tl
import time
class Frame:
    def __init__(self):
        self.head=0
        self.state=0
        self.len=0
        self.payload=b''
        self.crc=0
    def CRC8_MAXIN(self,data):
        crc = 0x00
        for byte in data:
            crc ^= byte
            for i in range(8):
                if crc & 0x01:
                    crc = (crc >> 1) ^ 0x8C
                else:
                    crc >>= 1
        return crc
    def receive(self,ser):
        timeout_start = time.time()
        while True:
            if time.time() - timeout_start > tl.WAITING_FRAME:
                tl.demoPrint("Timeout: No data received within the waiting time.")
                return False
            byte = ser.read(1)
            #print("Received byte: {}".format(byte))
            if not byte:
                continue
            if byte[0]!=0xAA:
                continue
            self.head=byte[0]
            self.state=ser.read(1)[0]
            self.len=ser.read(1)[0]
            self.payload=ser.read(self.len)
            self.crc=ser.read(1)[0]
            data=bytearray([self.head,self.state,self.len])+self.payload
            crc=self.CRC8_MAXIN(data)
            if crc==self.crc:
                return True
            else:
                tl.demoPrint("CRC error: expected {}, got {}".format(crc, self.crc))
                return False

class MessageBus:
    def __init__(self):
        self.rx_queue = queue.Queue()
        self.plot_queue = queue.Queue()
    def receive(self,ser):
        frame=Frame()
        if frame.receive(ser):
            self.rx_queue.put(frame)
    def outQueue(self):
        if not self.rx_queue.empty():
            result=self.rx_queue.get()
            return result.state,result.len,result.payload
        return None,None,None
    def plot_receive(self,data):
        self.plot_queue.put(data)
    def plot_get(self):
        if not self.plot_queue.empty():
            return self.plot_queue.get()
        return None