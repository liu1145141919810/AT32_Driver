import json
import os
class Translate:
    def __init__(self):
        _base = os.path.dirname(os.path.abspath(__file__))
        _can_dir = os.path.join(_base, "..", "..", "..", "CAN_midware")
        with open(os.path.join(_can_dir, "can_protocol.json")) as f:
            readfile=json.load(f)
            self.readfile_command=readfile["command"]
            self.readfile_report=readfile["report"]
        self.writefileH=os.path.join(_can_dir, "can_protocol.h")
        self.writefileC=os.path.join(_can_dir, "can_protocol.c")
    def translateH(self):
        with open(self.writefileH, "w") as f:
            f.write("#ifndef CAN_PROTOCOL_H\n")
            f.write("#define CAN_PROTOCOL_H\n")
            # transmit protocol
            for name,value in self.readfile_command.items():
                macro_name="CMD_"+name.upper()
                f.write(f"#define {macro_name} {value}\n")
            # receive protocol
            f.write("\n")
            for name,value in self.readfile_report.items():
                if isinstance(value,dict):
                    macro_name="RPT_"+value["state"].upper()+'_'+value["substate"].replace(" ", "_").upper()
                else:
                    macro_name="RPT_"+value.upper()
                f.write(f"#define {macro_name} {int(name)}\n")
            # Other definitions
            f.write("#endif // CAN_PROTOCOL_H\n")
if __name__=="__main__":
    translator=Translate()
    translator.translateH()