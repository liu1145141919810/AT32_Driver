import time
import sys
from .CommandBase import CommandBase,register_command,COMMAND_REGISTRY

from .. import publicTool as tl

class HostManager(CommandBase):
    def __init__(self,resManager):
        super().__init__(resManager)
        self.func=None
    def get_data(self):
        if self.com_mode=='uart':
            return self.resManager.usart_outQueue()
        else:
            pass
    def run(self):
        while True:
            self.checkExitFlag()
            state,length,payload=self.get_data()
            if payload is not None:
                last_time = time.time()  # Reset the last_time when a new payload is received
                tl.infoPrint(payload.decode(errors="ignore"))
                self.func = COMMAND_REGISTRY.get(tl.STATE_NUM[state])
                if self.func is not None:
                    self.func(self,state,length,payload)
            time.sleep(tl.CHECKING_DELAY)
    #========= Work FUnction Area==============
    @register_command("MONITOR")
    def HostMonitor(self,state,length,payload):
        temperature_data=[]
        vref_data=[]
        temp_str=b'internal_temperature = '
        vref_str=b'internal_vref = '
        while state==None or tl.STATE_NUM[state]=="MONITOR":
            time.sleep(tl.CHECKING_DELAY)
            self.checkExitFlag()
            
            #Blocking wait for new data to be available in the queue
            if state is None:
                state,length,payload=self.resManager.usart_outQueue()
                continue

            if payload[:len(temp_str)]==temp_str:
                temperature=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
                temperature_data.append(temperature)
            elif payload[:len(vref_str)]==vref_str:
                vref=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
                vref_data.append(vref)
            state,length,payload=self.resManager.usart_outQueue()

        self.resManager.plot_receive((temperature_data,vref_data))