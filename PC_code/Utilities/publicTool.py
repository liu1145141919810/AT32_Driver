WAITING_CONNECTING = 10 # seconds
WAITING_START = 10
WAITING_FRAME = 1
CHECKING_DELAY = 0.001 # seconds
DELAY_SCALE_FACTOR = 1000

PUBLIC_DEFAULT_STATE = 0

COMMUNICATION_MODE = "can" # "uart" or "can"
#CAN
CAN_CHANNEL="can0"
CAN_BITRATE=500000
def sysPrint(msg):
    print("\033[34m" + msg + "\033[0m")
def infoPrint(msg):
    print("\033[32m" + msg + "\033[0m")