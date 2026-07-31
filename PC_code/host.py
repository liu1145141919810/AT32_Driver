import serial
import argparse
import time # Using sleep function to avoid busy waiting
import select
import sys
WAITING_CONNECTING = 10 # seconds
WAITING_START = 10
CHECKING_DELAY = 0.001 # seconds
# ============== The Main Area =============#
def demoPrint(msg):
    print("\033[34m" + msg + "\033[0m")
def workarea(ser):
    while True:
        ready,_,_=select.select(
        [sys.stdin,ser],[],[]
        )
        for source in ready:
            if source == sys.stdin:
                tx_msg=sys.stdin.readline().rstrip()
                if(tx_msg == "HostExit"):
                    demoPrint("Host already exit the communication.")
                    return
                ser.write(
                    (tx_msg + "\n").encode()#patching the line change
                )
            elif source == ser:
                rx_msg=ser.readline().decode(errors="ignore").rstrip()
                demoPrint(rx_msg)
        time.sleep(CHECKING_DELAY)
#============= Communication Area =============#
def uart_communication(args):
    start_time = time.time()
    while True:
        try:
            ser = serial.Serial(
                port=args.port,
                baudrate=args.baudrate,
                timeout=1
            )
            break
        except serial.SerialException as e:
           pass
        if time.time() - start_time > WAITING_CONNECTING:
            demoPrint("Timeout: Could not open serial port within the waiting time.")
            return
        time.sleep(CHECKING_DELAY)
    
    demoPrint("Connection established.")
    #time.sleep(WAITING_FLASHING)
    demoPrint("Now waiting for the device to start...")
    while True:
        if ser.in_waiting:# Usually is acceptable to read after serval minutes
            msg=ser.readline().decode().rstrip()
            if(msg == "AT32_READY"):# The device side would send the etire sentence once a time, don't worry about reading the fragment
                demoPrint("Device start working successfully!")
                workarea(ser)
        if time.time() - start_time > WAITING_START:
            demoPrint("Timeout: Device did not start working within the waiting time.")
            return
        time.sleep(CHECKING_DELAY)

def can_communication(args):
    pass
def wifi_communication(args):
    pass
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Host script for serial communication")
    parser.add_argument("--port", default="/dev/ttyACM0", help="Serial port (default: /dev/ttyUSB0)")
    parser.add_argument("--baudrate", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--comMethod", default="uart", help="Communication method (default: uart)")
    args = parser.parse_args()
    if(args.comMethod == 'uart'):
        uart_communication(args)
    elif(args.comMethod == 'can'):
        can_communication(args)
    elif(args.comMethod == 'wifi'):
        wifi_communication(args)
    else:
        demoPrint("Invalid communication method.")