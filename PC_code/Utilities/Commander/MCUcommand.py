import os
import time
import json
import can

from .. import publicTool as tl
from .CommandBase import CommandBase
#for this, we only concern about can transmit
class MCUManager(CommandBase):
    def __init__(self,resManager):
        super().__init__(resManager)
        _base = os.path.dirname(os.path.abspath(__file__))
        _can_dir = os.path.join(_base, "..", "..", "..", "CAN_midware")
        with open(os.path.join(_can_dir, "can_commands.json")) as f:
            #tl.sysPrint("Loading command.json Successfully")
            self.command=json.load(f)
        with open(os.path.join(_can_dir, "can_protocol.json")) as f:
            #tl.sysPrint("Loading protocol.json Successfully")
            self.protocol=json.load(f)
    def run(self):
        while True:
            for item in self.command:
                self.checkExitFlag()# prepare for exit
                delay=item.get("delay", 0)
                cmd=item.get("Cmd",None)
                args=item.get("args",[])
                time.sleep(delay/tl.DELAY_SCALE_FACTOR)
                if cmd is not None:
                    id=self.protocol["command"].get(cmd,None)
                    if id is not None and args is not None:
                        msg=can.Message(
                            arbitration_id=id,
                            data=bytearray(args),
                            is_extended_id=False
                        )
                self.resManager.can_tx_receive(msg)
            tl.sysPrint("One epoch finished")