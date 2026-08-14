import time
import sys
import os
import json
import importlib
import glob
from .CommandBase import CommandBase,COMMAND_REGISTRY
from .CommandBase import CAN_TRANSLATION_REGISTRY

from .. import publicTool as tl

# ========= Auto-discover and import all registration modules =========
def _load_dealing_logic_modules():
    """Automatically import all modules in DealingLogic folder to trigger registration."""
    _base = os.path.dirname(os.path.abspath(__file__))
    _dealing_dir = os.path.join(_base, "..", "DealingLogic")
    _dealing_dir = os.path.abspath(_dealing_dir)
    if not os.path.isdir(_dealing_dir):
        return
    # Add parent to path if needed for relative imports
    _utilities_dir = os.path.join(_base, "..")
    if _utilities_dir not in sys.path:
        sys.path.insert(0, os.path.abspath(_utilities_dir))
    
    for py_file in glob.glob(os.path.join(_dealing_dir, "*.py")):
        module_name = os.path.splitext(os.path.basename(py_file))[0]
        if module_name.startswith("_"):
            continue
        try:
            # Import using the full module path for relative imports to work
            importlib.import_module(f"Utilities.DealingLogic.{module_name}")
        except Exception as e:
            print(f"[Warning] Failed to load DealingLogic module '{module_name}': {e}")
_load_dealing_logic_modules()

class HostManager(CommandBase):
    def __init__(self,resManager):
        super().__init__(resManager)
        self.func=None
        _base = os.path.dirname(os.path.abspath(__file__))
        _can_dir = os.path.join(_base, "..", "..", "..", "CAN_midware")
        with open(os.path.join(_can_dir, "can_protocol.json")) as f:
            self.protocol=json.load(f)
    def run(self):
        while True:
            self.checkExitFlag()
            state,length,payload=self.get_data()
            if state is not None:
                last_time = time.time()  # Reset the last_time when a new payload is received
                if payload is not None:
                    tl.infoPrint(payload.decode(errors="ignore"))
                self.func = COMMAND_REGISTRY.get(
                    self.get_state_name(state), None
                )
                if self.func is not None:
                    self.func(self,state,length,payload)
            time.sleep(tl.CHECKING_DELAY)
    #========= Common Usage  ==================
    def get_data(self):
        if self.com_mode=='uart': #============== for the uart mode===========
            return self.resManager.usart_outQueue()
        else:               # =================== for the can mode===========
            component=self.resManager.can_rx_transmit()
            if component is not None:
                tl.infoPrint("CAN Receive: ID=0x{:X}, Data={}".format(component.arbitration_id, component.data.hex()))
                protocol_head=self.protocol["report"].get(str(component.arbitration_id),None)
                if type(protocol_head)==dict:
                    main_state=protocol_head["state"]
                    sub_state=protocol_head["substate"]
                    data=component.data
                    self.func = CAN_TRANSLATION_REGISTRY.get(main_state,None)
                    return self.func(self,main_state,sub_state,data)
                elif type(protocol_head)==str:
                    state=self.protocol["state_num"][protocol_head]
                    length=0
                    payload=None
                    return state,length,payload
            return None,None,None
    def get_state_name(self,state):
        name=self.protocol["state_name"].get(str(state),None)
        return name