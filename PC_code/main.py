from Utilities.Communication import BaseCommunication
from Utilities.Commander import Hostcommand
from Utilities.Commander import MCUcommand
from Utilities.Communication import CommunicationManager as communi
import threading
import time
def argAnalyzer():
    import argparse
    parser = argparse.ArgumentParser(description="Host script for serial communication")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial port (default: /dev/ttyUSB0)")
    parser.add_argument("--baudrate", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--comMethod", default="uart", help="Communication method (default: uart)")
    args = parser.parse_args()
    return args
#Below is the data drawing function
def picture_one(data,name):
    import matplotlib.pyplot as plt
    fig,ax=plt.subplots(figsize=(10,5))
    ax.plot(data)
    ax.set_title(name)
    ax.legend()
    ax.set_xlabel("Time")
    ax.set_ylabel("Value")

    plt.show()

def drawData(resManager,threads):
    while True:
        #==     Getting Finished Telling ======
        if not any(thread.is_alive() for thread in threads):
            break
        plot_data = resManager.plot_get()
        if plot_data is not None:
            temperature_data, vref_data = plot_data
            t=time.time()
            t_str=time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(t))
            picture_one(temperature_data, "Temperature "+t_str)
            picture_one(vref_data, "VREF "+t_str)

if __name__ == "__main__":
    args=argAnalyzer()
    resManager = BaseCommunication.MessageBus()
    hostManager = Hostcommand.HostManager(resManager)
    mcuManager = MCUcommand.MCUManager(resManager)
    commuManager = communi.ComManager(
        args.port,
        args.baudrate,
        resManager,
        hostManager)

    thread1=threading.Thread(target=commuManager.run, args=())
    thread2=threading.Thread(target=hostManager.run, args=())
    threads = [thread1, thread2]
    thread1.start()
    thread2.start()
    drawData(resManager,threads)
    thread1.join()
    thread2.join()