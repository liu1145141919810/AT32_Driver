from Utilities.Communication import BaseCommunication
from Utilities.Commander import Hostcommand
from Utilities.Commander import MCUcommand
from Utilities.Communication import CommunicationManager as communi
from Utilities.Translate import Translate
from Utilities import publicTool as tl
from Data.dataLoader import DataLoader
import threading

def argAnalyzer():
    import argparse
    parser = argparse.ArgumentParser(description="Host script for serial communication")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial port (default: /dev/ttyUSB0)")
    parser.add_argument("--baudrate", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--comMethod", default="uart", help="Communication method (default: uart)")
    args = parser.parse_args()
    return args
if __name__ == "__main__":
    #Translate
    Translator=Translate.Translate()
    Translator.translateH1()
    Translator.translateC1()
    # Normal working flow running
    args=argAnalyzer()
    resManager = BaseCommunication.MessageBus()
    hostManager = Hostcommand.HostManager(resManager)
    mcuManager = MCUcommand.MCUManager(resManager)
    commuManager = communi.ComManager(
        args.port,
        args.baudrate,
        resManager,
        hostManager,
        mcuManager if tl.COMMUNICATION_MODE == "can" else None
        )

    thread1=threading.Thread(target=commuManager.run, args=())
    thread2=threading.Thread(target=hostManager.run, args=())
    threads = [thread1, thread2]
    thread1.start()
    thread2.start()
    if tl.COMMUNICATION_MODE == "can":
        thread3=threading.Thread(target=mcuManager.run, args=())
        threads.append(thread3)
        thread3.start()
    #Storing the relevant data
    data_loader = DataLoader(resManager, threads)
    data_loader.DataLoop()

    thread1.join()
    thread2.join()
    if tl.COMMUNICATION_MODE == "can":
        thread3.join()