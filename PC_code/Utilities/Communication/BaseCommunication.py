import queue
import time
from abc import ABC, abstractmethod

from .. import publicTool as tl
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
    def packup(self,state,tx_msg):
        self.head=0xAA
        self.state=state
        self.len=len(tx_msg)
        self.payload=tx_msg.encode()
        data=bytearray([self.head,self.state,self.len])+self.payload
        self.crc=self.CRC8_MAXIN(data)
        frame=data+bytearray([self.crc])
        return frame
    def receive(self,ser):
        timeout_start = time.time()
        while True:
            if time.time() - timeout_start > tl.WAITING_FRAME:
                tl.sysPrint("Timeout: No data received within the waiting time.")
                return False
            byte = ser.read(1)
            #print("Received byte: {}".format(byte))
            if not byte:
                continue
            if byte[0]!=0xAA:
                continue
            try:
                self.head=byte[0]
                self.state=ser.read(1)[0]
                self.len=ser.read(1)[0]
                self.payload=ser.read(self.len)
                self.crc=ser.read(1)[0]
            except Exception as e:
                tl.sysPrint("Error occurred while reading from serial port: {}".format(e))
                return False
            data=bytearray([self.head,self.state,self.len])+self.payload
            crc=self.CRC8_MAXIN(data)
            if crc==self.crc:
                return True
            else:
                tl.sysPrint("CRC error: expected {}, got {}".format(crc, self.crc))
                return False

            
class Communication(ABC):
    @abstractmethod
    def __init__(self):
        pass
    @abstractmethod
    def prework(self):
        pass
    @abstractmethod
    def tell_transmit(self):
        pass
    @abstractmethod
    def tell_receive(self):
        pass
    @abstractmethod
    def transmit(self):
        pass
    @abstractmethod
    def receive(self):
        pass

class MessageBus:
    def __init__(self):
        self.rx_queue = queue.Queue()
        self.plot_queue = queue.Queue()
    def usart_receive(self,ser):
        frame=Frame()
        if frame.receive(ser):
            self.rx_queue.put(frame)
    def usart_outQueue(self):
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
    #Packup transmission data into byyestream
    def frameup(self,state,tx_msg):
        frame=Frame()
        return frame.packup(state,tx_msg)