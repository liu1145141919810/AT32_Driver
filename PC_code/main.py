from Utilities import messageBus, commandManager, uartCommunication
import threading
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
    i=0;
    while True:
        #==     Getting Finished Telling ======
        if not any(thread.is_alive() for thread in threads):
            break
        plot_data = resManager.plot_get()
        if plot_data is not None:
            temperature_data, vref_data = plot_data
            picture_one(temperature_data, "Temperature"+str(i))
            picture_one(vref_data, "VREF"+str(i))
        i+=1

if __name__ == "__main__":
    args=argAnalyzer()
    resManager = messageBus.MessageBus()
    cmdManager = commandManager.CommandManager(resManager)
    uartManager = uartCommunication.UartCommunication(
        args.port, 
        args.baudrate,
        resManager,
        cmdManager)

    thread1=threading.Thread(target=uartManager.run, args=())
    thread2=threading.Thread(target=cmdManager.run, args=())
    threads = [thread1, thread2]
    thread1.start()
    thread2.start()
    drawData(resManager,threads)
    thread1.join()
    thread2.join()