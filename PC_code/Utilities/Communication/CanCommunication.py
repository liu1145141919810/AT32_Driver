import time
import can
import sys

sys.path.append("..")
from .. import publicTool as tl
from .BaseCommunication import Communication
class CanCommunication(Communication):
    def __init__(self,resManager,hostManager,mcuManager):
        self.resManager = resManager
        self.hostManager = hostManager
        self.mcuManager = mcuManager
        self._rx_cache = None
        import subprocess
        subprocess.run(
            ["sudo","ip","link","set","can0","down"]
        )
        subprocess.run(
            ["sudo","ip","link","set","can0","up"]
        )
        self.bus=can.Bus(
            channel=tl.CAN_CHANNEL,
            interface="socketcan",
            bitrate=tl.CAN_BITRATE
        )
        tl.sysPrint("CAN Communication initialized successfully.")
    def exitPreTelling(self):
        pass
    # ======== the standard format is following ==========
    def prework(self):
       pass
    def tell_transmit(self):
        if self.resManager.can_queue_tx.empty():
            return False
        return True
    def tell_receive(self):
        self._rx_cache = self.bus.recv(timeout=0.1)
        if self._rx_cache is not None:
            return True
        return False
    def transmit(self):
        self.exitPreTelling()
        msg=self.resManager.can_tx_transmit()
        if msg is not None:
            self.bus.send(msg)
    def receive(self):
        msg = self._rx_cache
        self._rx_cache = None
        if msg is not None:
            self.resManager.can_rx_receive(msg)
if __name__ == "__main__":
    can_comm=CanCommunication(None,None,None)
    tl.sysPrint("CanCommunication test Successfully!")