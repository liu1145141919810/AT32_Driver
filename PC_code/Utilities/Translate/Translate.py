import json
import os
class Translate:
    def __init__(self):
        _base = os.path.dirname(os.path.abspath(__file__))
        _can_dir = os.path.join(_base, "..", "..", "..", "CAN_midware")
        with open(os.path.join(_can_dir, "can_protocol.json")) as f:
            self.readfile=json.load(f)["command"]
        self.writefile=os.path.join(_can_dir, "can_protocol.h")
    def translate(self):
        with open(self.writefile, "w") as f:
            f.write("#ifndef CAN_PROTOCOL_H\n")
            f.write("#define CAN_PROTOCOL_H\n")
            for name,value in self.readfile.items():
                macro_name="CMD_"+name.upper()
                f.write(f"#define {macro_name} {value}\n")
            f.write("\n#endif\n")
if __name__=="__main__":
    translator=Translate()
    translator.translate()