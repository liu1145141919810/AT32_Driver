from . import publicTool as tl
import time
import sys
class CommandManager:
    def __init__(self,resManager):
        self.resManager = resManager
        self.state_to_func={
            "DEFAULT":None,
            "ORDER":None,
            "LIGHT":None,
            "MONITOR":self.HostMonitor,
            "BRIGHT":None,
            "CALIBRATE":None,
            "NOADDING":None,
            "ERROR_EVENT":None,
            "ERROR_STATE":None,
        }
        self.func=None
        self.exitFlag = False
    def setExitFlag(self):
        self.exitFlag = True
    def checkExitFlag(self):
        if self.exitFlag:
            tl.demoPrint("Exiting CommandManager run loop.")
            sys.exit(0)
    def run(self):
        while True:
            self.checkExitFlag()
            state,length,payload=self.resManager.outQueue()
            if payload is not None:
                last_time = time.time()  # Reset the last_time when a new payload is received
                tl.infoPrint(payload.decode(errors="ignore"))
                if self.state_to_func[tl.STATE_NUM[state]] is not None:
                    self.func=self.state_to_func[tl.STATE_NUM[state]]
                    self.func(state,length,payload)
            time.sleep(tl.CHECKING_DELAY)
    #========= Work FUnction Area==============
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
                state,length,payload=self.resManager.outQueue()
                continue

            if payload[:len(temp_str)]==temp_str:
                temperature=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
                temperature_data.append(temperature)
            elif payload[:len(vref_str)]==vref_str:
                vref=float(payload.decode(errors="ignore").split("=")[1].split(" ")[1])
                vref_data.append(vref)
            state,length,payload=self.resManager.outQueue()

        self.resManager.plot_receive((temperature_data,vref_data))