WAITING_CONNECTING = 10 # seconds
WAITING_START = 10
WAITING_FRAME = 1
CHECKING_DELAY = 0.001 # seconds
PUBLIC_DEFAULT_STATE = 0
STATE_NUM={
    0:"DEFAULT",
    1:"ORDER",
    2:"LIGHT",
    3:"MONITOR",
    4:"BRIGHT",
    5:"CALIBRATE",
    6:"NOADDING",
    7:"ERROR_EVENT",
    8:"ERROR_STATE",
}
def demoPrint(msg):
    print("\033[34m" + msg + "\033[0m")
def infoPrint(msg):
    print("\033[32m" + msg + "\033[0m")