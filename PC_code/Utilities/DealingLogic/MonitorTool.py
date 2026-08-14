from ..Commander.CommandBase import register_can_translation, CAN_TRANSLATION_REGISTRY
from ..Commander.CommandBase import CommandBase, register_command, COMMAND_REGISTRY
import time
from .. import publicTool as tl
@register_command("MONITOR")
def HostMonitor(self,state,length,payload):
    temperature_data=[]
    vref_data=[]
    temp_str=b'internal_temperature'
    vref_str=b'internal_vref'
    while state==None or self.get_state_name(state)=="MONITOR":
        time.sleep(tl.CHECKING_DELAY)
        self.checkExitFlag()
        
        #Blocking wait for new data to be available in the queue
        if state is None or payload is None:
            state,length,payload=self.get_data()
            continue

        if payload[:len(temp_str)]==temp_str:
            temperature=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
            temperature_data.append(temperature)
        elif payload[:len(vref_str)]==vref_str:
            vref=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
            vref_data.append(vref)
        state,length,payload=self.get_data()

    self.resManager.plot_receive((temperature_data,vref_data))
#========== Can Analysis Function Area=========
@register_can_translation("MONITOR")
def CanMonitor(self,main_state,substate,data):
    #The first byte is data length for param, so discard when anaalyzing the data
    length=data[0]
    data=data[1:]
    state=self.protocol["state_num"][main_state]
    if substate=="internal_temperature":
        temperature=int.from_bytes(data,byteorder='little',signed=True)/100
        sentence="internal_temperature = {} C".format(temperature)
        return state,length,sentence.encode()
    elif substate=="internal_vref":
        vref=int.from_bytes(data,byteorder='little',signed=False)/1000
        sentence="internal_vref = {} V".format(vref)
        return state,length,sentence.encode()