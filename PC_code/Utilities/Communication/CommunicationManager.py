import time

from .. import publicTool as tl
from .UartCommunication import UartCommunication
from .BaseCommunication import MessageBus
from .CanCommunication import CanCommunication
class ComManager:
    def communication(self,port,baudrate,resManager,hostManager,mcuManager=None):
        communicationManager=None
        method=tl.COMMUNICATION_MODE
        if method=="uart":
            try:
                communicationManager = UartCommunication(
                    port, 
                    baudrate,
                    resManager,
                    hostManager,)
            except Exception as e:
                try:
                    tl.sysPrint("Error initializing UART communication, using CAN")
                    communicationManager = CanCommunication(
                        resManager,
                        hostManager,
                        mcuManager)
                except Exception as e:
                    tl.sysPrint("All falied. Exiting...")
                    exit(1)
        
        elif method=="can":
            try:
                communicationManager = CanCommunication(
                    resManager,
                    hostManager,
                    mcuManager)
            except Exception as e:
                tl.sysPrint("Error initializing CAN communication,using UART")
                try:
                    communicationManager = UartCommunication(
                        port, 
                        baudrate,
                        resManager,
                        hostManager)
                except Exception as e:
                    tl.sysPrint("All falied. Exiting...")
                    exit(1)
        return communicationManager
    def __init__(self,port,baudrate,resManager,hostManager,mcuManager=None):
        self.comm=self.communication(port,baudrate,resManager,hostManager,mcuManager)
        self.message_bus = MessageBus()
    def prework(self):
        self.comm.prework();
    def run(self):
        self.prework();
        tl.sysPrint("Communication Manager is running...")
        while True:
            if self.comm.tell_transmit():
                self.comm.transmit()
            elif self.comm.tell_receive():
                self.comm.receive()
            time.sleep(tl.CHECKING_DELAY)