import serial
import argparse
import time # Using sleep function to avoid busy waiting
import select
import sys
WAITING_CONNECTING = 10 # seconds
WAITING_START = 10
WAITING_FRAME = 1
CHECKING_DELAY = 0.001 # seconds
def demoPrint(msg):
    print("\033[34m" + msg + "\033[0m")
def infoPrint(msg):
    print("\033[32m" + msg + "\033[0m")
class Frame:
    def __init__(self):
        self.head=0
        self.state=0
        self.len=0
        self.payload=b''
        self.crc=0
    def CRC8_MAXIN(self,data):
        crc = 0x00
        for byte in data:
            crc ^= byte
            for i in range(8):
                if crc & 0x01:
                    crc = (crc >> 1) ^ 0x8C
                else:
                    crc >>= 1
        return crc
    def receive(self,ser):
        timeout_start = time.time()
        while True:
            if time.time() - timeout_start > WAITING_FRAME:
                demoPrint("Timeout: No data received within the waiting time.")
                return False
            byte = ser.read(1)
            #print("Received byte: {}".format(byte))
            if not byte:
                continue
            if byte[0]!=0xAA:
                continue
            self.head=byte[0]
            self.state=ser.read(1)[0]
            self.len=ser.read(1)[0]
            self.payload=ser.read(self.len)
            self.crc=ser.read(1)[0]
            data=bytearray([self.head,self.state,self.len])+self.payload
            crc=self.CRC8_MAXIN(data)
            if crc==self.crc:
                return True
            else:
                demoPrint("CRC error: expected {}, got {}".format(crc, self.crc))
                return False
def handle_Frame(frame):
    infoPrint(frame.payload.decode(errors="ignore"))

# ============== The Main Area =============#
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
            elif source == ser:# Dueto it's bytestream, there is no need to worry about continuous fragment lossing
                frame=Frame()
                if frame.receive(ser):
                    handle_Frame(frame)

        time.sleep(CHECKING_DELAY)
#============= Communication Area =============#
def uart_communication(args):
    start_time = time.time()
    while True:
        try:
            ser = serial.Serial(
                port=args.port,
                baudrate=args.baudrate,
                timeout=1)
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
            demoPrint("Waiting Stablity...")
            frame=Frame()
            if frame.receive(ser):
                demoPrint("Waiting Fishished")
                if frame.payload.decode(errors="ignore") == "AT32_READY\r\n":# The device side would send the etire sentence once a time, don't worry about reading the fragment
                    demoPrint("Device start working successfully!")
                    workarea(ser)
                    return
                else:
                    demoPrint("Startup Failed, please check the device.")
                    

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