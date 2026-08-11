import time
import json

from .. import publicTool as tl
from .CommandBase import CommandBase
#for this, we only concern about can transmit
class MCUManager(CommandBase):
    def __init__(self,resManager):
        super().__init__(resManager)
        with open("../../../CAN_midware/can_command.json") as f:
            tl.sysPrint("Loading command.json Successfully")
            self.command=json.load(f)
        with open("../../../CAN_midware/can_protocol.json") as f:
            tl.sysPrint("Loading protocol.json Successfully")
            self.protocol=json.load(f)
    def run(self):
        while True:
            for item in self.command:
                delay=item.get("delay", 0)
                cmd=item.get("Cmd",None)
                args=item.get("args",[])
                time.sleep(delay)
                if cmd is not None:
                    pass

