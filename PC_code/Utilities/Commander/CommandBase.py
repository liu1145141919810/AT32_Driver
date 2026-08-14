from abc import ABC, abstractmethod
from typing import final

import sys

from .. import publicTool as tl

#======== Registered file for Host Function===================
COMMAND_REGISTRY={}
def register_command(name):
    def decorator(func):
        COMMAND_REGISTRY[name] = func
        return func
    return decorator
#======== Registered file for Host received can translation===================
CAN_TRANSLATION_REGISTRY={}
def register_can_translation(name):
    def decorator(func):
        CAN_TRANSLATION_REGISTRY[name] = func
        return func
    return decorator
#=================================#
class CommandBase(ABC):
    def __init__(self,resManager):
        self.resManager=resManager
        self.exitFlag=False
        self.com_mode=tl.COMMUNICATION_MODE
    @final
    def setExitFlag(self):
        self.exitFlag=True
    @final
    def checkExitFlag(self):
        if self.exitFlag:
            tl.sysPrint("Exiting CommandManager run loop.")
            sys.exit(0)
    @abstractmethod
    def run(self):
        pass